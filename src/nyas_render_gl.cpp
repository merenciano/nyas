
#include "nyas.h"
#include "nyas_render.h"

#include <glad/glad.h>
#include <stdio.h>

static const GLint attrib_sizes[NyasVtxAttrib_COUNT] = { 3, 3, 3, 3, 2, 4 };
static const char *attrib_names[NyasVtxAttrib_COUNT] = { "a_position", "a_normal", "a_tangent",
    "a_bitangent", "a_uv", "a_color" };

static GLint _GL_TexFilter(NyasTexFilter f)
{
    switch (f)
    {
        case NyasTexFilter_Linear: return GL_LINEAR;
        case NyasTexFilter_LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
        case NyasTexFilter_LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
        case NyasTexFilter_Nearest: return GL_NEAREST;
        case NyasTexFilter_NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case NyasTexFilter_NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
        default: return 0;
    }
}

static GLint _GL_TexWrap(NyasTexWrap w)
{
    switch (w)
    {
        case NyasTexWrap_Repeat: return GL_REPEAT;
        case NyasTexWrap_Clamp: return GL_CLAMP_TO_EDGE;
        case NyasTexWrap_Mirror: return GL_MIRRORED_REPEAT;
        case NyasTexWrap_Black: return GL_CLAMP_TO_BORDER;
        default: return 0;
    }
}

struct NyGL_TexConfig
{
    GLenum MinFltr        = GL_LINEAR;
    GLenum MagFltr        = GL_LINEAR;
    GLenum WrapS          = GL_CLAMP_TO_EDGE;
    GLenum WrapT          = GL_CLAMP_TO_EDGE;
    GLenum WrapR          = GL_CLAMP_TO_EDGE;
    float  BorderColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    NyGL_TexConfig(NyasSampler *t) :
        MinFltr(_GL_TexFilter(t->MinFilter)), MagFltr(_GL_TexFilter(t->MagFilter)),
        WrapS(_GL_TexWrap(t->WrapS)), WrapT(_GL_TexWrap(t->WrapT)), WrapR(_GL_TexWrap(t->WrapR))
    {
    }
};

struct _GL_Format
{
    GLint  InternalFormat;
    GLenum Format;
    GLenum Type;
};

static inline _GL_Format _GL_TexFmt(NyasTexFmt Format)
{
    switch (Format)
    {
        case NyasTexFmt_R_8: return { GL_R8, GL_RED, GL_UNSIGNED_BYTE };
        case NyasTexFmt_RG_8: return { GL_RG8, GL_RG, GL_UNSIGNED_BYTE };
        case NyasTexFmt_RGB_8: return { GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE };
        case NyasTexFmt_RGBA_8: return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE };
        case NyasTexFmt_SRGB_8: return { GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE };
        case NyasTexFmt_R_16F: return { GL_R16F, GL_RED, GL_HALF_FLOAT };
        case NyasTexFmt_RG_16F: return { GL_RG16F, GL_RG, GL_HALF_FLOAT };
        case NyasTexFmt_RGB_16F: return { GL_RGB16F, GL_RGB, GL_HALF_FLOAT };
        case NyasTexFmt_RGBA_16F: return { GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT };
        case NyasTexFmt_RGB_32F: return { GL_RGB32F, GL_RGB, GL_FLOAT };
        case NyasTexFmt_Depth: return { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT };
        case NyasTexFmt_DepthStencil: return { GL_DEPTH24_STENCIL8, GL_DEPTH_COMPONENT, GL_FLOAT };
        default: printf("Unrecognized texture format: (%d).", Format); return { 0, 0, 0 };
    }
}

static GLenum _GL_ShaderType(NyasShaderStage stage)
{
    switch (stage)
    {
        case NyasShaderStage_Vertex: return GL_VERTEX_SHADER;
        case NyasShaderStage_Fragment: return GL_FRAGMENT_SHADER;
        default: assert(false);
    }
}

namespace nyas::render
{
void _NySetProcLoader(void *(*load_fn)(const char *))
{
    gladLoadGLLoader(load_fn);
}

void _NyCreateTex(NyasTexFlags flags, int unit, NyResourceID *out_id, int count)
{
    GLenum t = (flags & NyasTexFlags_Cubemap) ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_2D_ARRAY;
    glCreateTextures(t, count, out_id);
    glBindTextures(unit, count, out_id);
}

void _NyAllocTex(NyResourceID id, NyasTexInfo info, int layers)
{
    auto fmt = _GL_TexFmt(info.Format);
    glTextureStorage3D(id, info.Levels, fmt.InternalFormat, info.Width, info.Height, layers);
}

void _NySetTex(NyResourceID id, NyasTexture tex, NyasTexImage img, NyasTexInfo info, int faces)
{
    auto fmt = _GL_TexFmt(info.Format);
    for (int i = 0; i < faces; ++i)
    {
        glTextureSubImage3D(id, img.Level, 0, 0, tex.Layer * faces + i, info.Width >> img.Level,
            info.Height >> img.Level, 1, fmt.Format, fmt.Type, img.Data[i]);
        NYAS_FREE(img.Data[i]);
    }
}

void _NyCreatePipeline(NyResourceID *id, NyasPipeline *pipeline)
{
    *id = glCreateProgram();
    glGenBuffers(1, &pipeline->DataID);
    glBindBuffer(GL_UNIFORM_BUFFER, pipeline->DataID);
    glBufferData(GL_UNIFORM_BUFFER, pipeline->DataSize, pipeline->Data, GL_DYNAMIC_DRAW);
}

void _NyBuildPipeline(NyResourceID id, NyasPipelineBuilder *pb)
{
    NyResourceID shader_ids[NyasShaderStage_COUNT];
    GLchar out_log[2048];
    GLint err;
    for (int i = 0; i < NyasShaderStage_COUNT; ++i)
    {
        if (pb->Source[i].SrcStr.empty()) 
        {
            shader_ids[i] = NYAS_INVALID_RESOURCE_ID;
            continue;
        }

        shader_ids[i] = glCreateShader(_GL_ShaderType(i));
        glShaderSource(shader_ids[i], pb->Source[i].SrcStr.size(), pb->Source[i].SrcStr.data(), pb->Source[i].SrcLen.data());
        glCompileShader(shader_ids[i]);
        glGetShaderiv(shader_ids[i], GL_COMPILE_STATUS, &err);
        if (!err)
        {
            glGetShaderInfoLog(shader_ids[i], 2048, NULL, out_log);
            NYAS_LOG_ERR("Stage(%d):\n%s\n", i, out_log);
        }

        glAttachShader(id, shader_ids[i]);
    }

    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &err);
    if (!err)
    {
        glGetProgramInfoLog(id, 2048, NULL, out_log);
        NYAS_LOG_ERR("Program link error:\n%s\n", out_log);
    }

    for (int i = 0; i < NyasShaderStage_COUNT; ++i)
    {
        if (shader_ids[i] != NYAS_INVALID_RESOURCE_ID)
        {
            glDetachShader(id, shader_ids[i]);
            glDeleteShader(shader_ids[i]);
        }
    }
}

void _NyUsePipeline(NyResourceID id, NyasPipeline pipeline)
{
    glUseProgram(id);
    if (pipeline.DataSize)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, pipeline.DataID);
        glBufferData(GL_UNIFORM_BUFFER, pipeline.DataSize,
            pipeline.Data, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, pipeline.DataID);
    }
}

void _NyCreateMesh(uint32_t *id, uint32_t *vid, uint32_t *iid)
{
    glGenVertexArrays(1, id);
    glGenBuffers(1, vid);
    glGenBuffers(1, iid);
}

void _NyUseMesh(NyasMesh *m)
{
    if (m)
    {
        glBindVertexArray(m->Resource.ID);
    }
    else
    {
        glBindVertexArray(0);
    }
}

static GLsizei _GetAttribStride(int32_t attr_flags)
{
    GLsizei stride = 0;
    for (int i = 0; i < NyasVtxAttrib_COUNT; ++i)
    {
        if (attr_flags & (1 << i))
        {
            stride += attrib_sizes[i];
        }
    }
    return stride * sizeof(float);
}

void _NySetMesh(NyasMesh *mesh, uint32_t shader_id)
{
    glBindVertexArray(mesh->Resource.ID);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->ResVtx.ID);
    glBufferData(GL_ARRAY_BUFFER, mesh->VtxSize, mesh->Vtx, GL_STATIC_DRAW);

    GLint   offset = 0;
    GLsizei stride = _GetAttribStride(mesh->Attribs);
    for (int i = 0; i < NyasVtxAttrib_COUNT; ++i)
    {
        if (!(mesh->Attribs & (1 << i)))
        {
            continue;
        }

        GLint size       = attrib_sizes[i];
        GLint attrib_pos = glGetAttribLocation(shader_id, attrib_names[i]);
        if (attrib_pos >= 0)
        {
            glEnableVertexAttribArray(attrib_pos);
            glVertexAttribPointer(
                attrib_pos, size, GL_FLOAT, GL_FALSE, stride, (void *)(offset * sizeof(float)));
        }
        offset += size;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ResIdx.ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->ElementCount * sizeof(NyDrawIdx),
        (const void *)mesh->Indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void _NyReleaseMesh(uint32_t *id, uint32_t *vid, uint32_t *iid)
{
    glDeleteVertexArrays(1, id);
    glDeleteBuffers(1, vid);
    glDeleteBuffers(1, iid);
}

void _NyCreateFramebuf(NyasFramebuffer *fb)
{
    glGenFramebuffers(1, &fb->Resource.ID);
}

int _GL_FramebufAttach(NyasFbAttach a)
{
    switch (a)
    {
        case NyasFbAttach_Depth: return GL_DEPTH_ATTACHMENT;
        case NyasFbAttach_Stencil: return GL_STENCIL_ATTACHMENT;
        case NyasFbAttach_DepthStencil: return GL_DEPTH_STENCIL_ATTACHMENT;
        default: return GL_COLOR_ATTACHMENT0 + a;
    }
}

void _NySetFramebuf(uint32_t fb_id, NyasTexTarget *tt)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
    GLint slot = _GL_FramebufAttach(tt->Attach);
    glFramebufferTextureLayer(
        GL_FRAMEBUFFER, slot, GTextures.TexInternalIDs[tt->Tex.Index], tt->MipLevel, tt->Tex.Layer);
}

void _NyUseFramebuf(uint32_t id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void _NyReleaseFramebuf(NyasFramebuffer *fb)
{
    glDeleteFramebuffers(1, &fb->Resource.ID);
}

void _NyClear(bool color, bool depth, bool stencil)
{
    GLbitfield mask = 0;
    mask |= (GL_COLOR_BUFFER_BIT * color);
    mask |= (GL_DEPTH_BUFFER_BIT * depth);
    mask |= (GL_STENCIL_BUFFER_BIT * stencil);
    if (mask)
    {
        glClear(mask);
    }
}

void _NyDraw(int elem_count, int index_type, int instances)
{
    glDrawElementsInstanced(
        GL_TRIANGLES, elem_count, index_type ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0, instances);
}

void _NyClearColor(float r, float g, float b, float a)
{
    if (a >= 0.0f)
    {
        glClearColor(r, g, b, a);
    }
}

void _NyEnableBlend(void)
{
    glEnable(GL_BLEND);
}

void _NyDisableBlend(void)
{
    glDisable(GL_BLEND);
}

void _NyEnableCull(void)
{
    glEnable(GL_CULL_FACE);
}

void _NyDisableCull(void)
{
    glDisable(GL_CULL_FACE);
}

void _NyEnableDepthTest(void)
{
    glEnable(GL_DEPTH_TEST);
}

void _NyDisableDepthTest(void)
{
    glDisable(GL_DEPTH_TEST);
}

void _NyEnableDepthMask(void)
{
    glDepthMask(GL_TRUE);
}

void _NyDisableDepthMask(void)
{
    glDepthMask(GL_FALSE);
}

static GLenum _GL_Blend(NyasBlendFunc blend_func)
{
    switch (blend_func)
    {
        case NyasBlendFunc_Default: return 0xFFFF;
        case NyasBlendFunc_One: return GL_ONE;
        case NyasBlendFunc_SrcAlpha: return GL_SRC_ALPHA;
        case NyasBlendFunc_OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case NyasBlendFunc_Zero: return GL_ZERO;
        default: NYAS_ASSERT(false); return -1;
    }
}

void _NySetBlend(NyasBlendFunc blend_func_src, NyasBlendFunc blend_func_dst)
{
    GLenum gl_src = _GL_Blend(blend_func_src);
    if (gl_src != 0xFFFF)
    {
        glBlendFunc(gl_src, _GL_Blend(blend_func_dst));
    }
}

static GLenum _GL_Cull(NyasFaceCull cull)
{
    switch (cull)
    {
        case NyasFaceCull_Back: return GL_BACK;
        case NyasFaceCull_Front: return GL_FRONT;
        case NyasFaceCull_FrontAndBack: return GL_FRONT_AND_BACK;
        case NyasFaceCull_Default: return 0;
        default: return NyasError_SwitchBadLabel;
    }
}

void _NySetCull(NyasFaceCull cull_face)
{
    GLenum gl_value = _GL_Cull(cull_face);
    if (gl_value)
    {
        glCullFace(_GL_Cull(cull_face));
    }
}

static GLenum _GL_DepthFunc(NyasDepthFunc df)
{
    switch (df)
    {
        case NyasDepthFunc_LessEqual: return GL_LEQUAL;
        case NyasDepthFunc_Less: return GL_LESS;
        case NyasDepthFunc_Default: return 0;
        default: return NyasError_SwitchBadLabel;
    }
}

void _NySetDepthFunc(NyasDepthFunc depth_func)
{
    GLenum gl_value = _GL_DepthFunc(depth_func);
    if (gl_value)
    {
        glDepthFunc(gl_value);
    }
}

void _NyEnableStencilTest(void)
{
    glEnable(GL_STENCIL);
}

void _NyDisableStencilTest(void)
{
    glDisable(GL_STENCIL);
}

void _NyEnableStencilMask(void)
{
    glStencilMask(GL_TRUE);
}

void _NyDisableStencilMask(void)
{
    glStencilMask(GL_FALSE);
}

void _NyEnableScissor(void)
{
    glEnable(GL_SCISSOR_TEST);
}

void _NyDisableScissor(void)
{
    glDisable(GL_SCISSOR_TEST);
}

void _NyViewport(NyRect rect)
{
    if (rect.X != rect.W)
    {
        glViewport(rect.X, rect.Y, rect.W, rect.H);
    }
}

void _NyScissor(NyRect rect)
{
    if (rect.X != rect.W)
    {
        glScissor(rect.X, rect.Y, rect.W, rect.H);
    }
}
} // namespace nyas::render