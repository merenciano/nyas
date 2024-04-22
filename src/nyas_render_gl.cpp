
#include "nyas_render.h"
#include "nyas.h"

#include <stdio.h>
#include <glad/glad.h>

static const GLint attrib_sizes[NyasVtxAttrib_COUNT] = { 3, 3, 3, 3, 2, 4 };
static const char *attrib_names[NyasVtxAttrib_COUNT] = { "a_position", "a_normal", "a_tangent",
    "a_bitangent", "a_uv", "a_color" };

struct NyGL_TexDesc
{
    GLenum target;
    GLint min_f;
    GLint mag_f;
    GLint ws;
    GLint wt;
    GLint wr;
    float br;
    float bg;
    float bb;
    float ba;

    NyGL_TexDesc(GLenum tg, GLint min, GLint mag, GLint ws, GLint wt, GLint wr, float br, float bg,
        float bb, float ba) :
        target(tg),
        min_f(min), mag_f(mag), ws(ws), wt(wt), wr(wr), br(br), bg(bg), bb(bb), ba(ba)
    {
    }
};

static GLenum _GL_TexTarget(NyasTexType type)
{
    switch (type)
    {
        case NyasTexType_2D: return GL_TEXTURE_2D;
        case NyasTexType_Cubemap: return GL_TEXTURE_CUBE_MAP;
        case NyasTexType_Array2D: return GL_TEXTURE_2D_ARRAY;
        case NyasTexType_ArrayCubemap: return GL_TEXTURE_CUBE_MAP_ARRAY;
        default: return 0;
    }
}

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
    GLenum MinFltr = GL_LINEAR;
    GLenum MagFltr = GL_LINEAR;
    GLenum WrapS = GL_CLAMP_TO_EDGE;
    GLenum WrapT = GL_CLAMP_TO_EDGE;
    GLenum WrapR = GL_CLAMP_TO_EDGE;
    float BorderColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    NyGL_TexConfig(NyasTexture *t)
        : MinFltr(_GL_TexFilter(t->Sampler.MinFilter))
        , MagFltr(_GL_TexFilter(t->Sampler.MagFilter))
        , WrapS(_GL_TexWrap(t->Sampler.WrapS))
        , WrapT(_GL_TexWrap(t->Sampler.WrapT))
        , WrapR(_GL_TexWrap(t->Sampler.WrapR)) {}
};

struct _GL_TexFmtResult
{
    GLint ifmt;
    GLenum fmt;
    GLenum type;
};

static struct _GL_TexFmtResult _GL_TexFmt(NyasTexFmt fmt)
{
    switch (fmt)
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
        case NyasTexFmt_Depth: return { GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT };
        default: NYAS_LOG_ERR("Unrecognized texture format: (%d).", fmt); return { 0, 0, 0 };
    }
}

namespace nyas::render
{
    void _NyCreateTex(NyasTexture *t)
    {
        auto tgt = _GL_TexTarget(t->Data.Type);
        NyGL_TexConfig smplr = {t};
        _GL_TexFmtResult fmt = _GL_TexFmt(t->Data.Format);
        glGenTextures(1, &t->Resource.Id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(tgt, t->Resource.Id);
        if (smplr.MinFltr)
        {
            glTexParameteri(tgt, GL_TEXTURE_MIN_FILTER, smplr.MinFltr);
        }
        if (smplr.MagFltr)
        {
            glTexParameteri(tgt, GL_TEXTURE_MAG_FILTER, smplr.MagFltr);
        }
        if (smplr.WrapS)
        {
            glTexParameteri(tgt, GL_TEXTURE_WRAP_S, smplr.WrapS);
        }
        if (smplr.WrapT)
        {
            glTexParameteri(tgt, GL_TEXTURE_WRAP_T, smplr.WrapT);
        }
        if (smplr.WrapR)
        {
            glTexParameteri(tgt, GL_TEXTURE_WRAP_R, smplr.WrapR);
        }
        if (smplr.BorderColor[3] > 0.0f)
        {
            glTexParameterfv(tgt, GL_TEXTURE_BORDER_COLOR, &smplr.BorderColor[0]);
        }
    }

    void _NySetTex(struct NyasTexture *t)
    {
        GLenum type = _GL_TexTarget(t->Data.Type);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(type, t->Resource.Id);
        struct _GL_TexFmtResult fmt = _GL_TexFmt(t->Data.Format);

        if (type == GL_TEXTURE_2D_ARRAY)
        {
            assert(false);
            for (int i = 0; i < t->Img.Size; ++i)
            {
                int w = t->Data.Width >> t->Img[i].MipLevel;
                int h = t->Data.Height >> t->Img[i].MipLevel;
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, t->Img[i].MipLevel, 0, 0, t->Img[i].Index, w, h, 1, fmt.fmt, fmt.type, t->Img[i].Pix);
            }
        }
        else
        {
            for (int i = 0; i < t->Img.Size; ++i)
            {
                GLint target = (t->Data.Type == NyasTexType_2D) ?
                                GL_TEXTURE_2D :
                                GL_TEXTURE_CUBE_MAP_POSITIVE_X + t->Img[i].Face;
                int w = t->Data.Width >> t->Img[i].MipLevel;
                int h = t->Data.Height >> t->Img[i].MipLevel;
                glTexImage2D(
                    target, t->Img[i].MipLevel, fmt.ifmt, w, h, 0, fmt.fmt, fmt.type, t->Img[i].Pix);
            }
        }

        if (t->Data.Flags & NyasTexFlags_GenMipMaps)
        {
            glGenerateMipmap(type);
        }
    }

    void _NyReleaseTex(uint32_t *id)
    {
        glDeleteTextures(1, id);
    }

    void _NyCreateMesh(uint32_t *id, uint32_t *vid, uint32_t *iid)
    {
        glGenVertexArrays(1, id);
        glGenBuffers(1, vid);
        glGenBuffers(1, iid);
    }

    void _NyUseMesh(NyasMesh *m, NyasShader *s)
    {
        (void)s;
        if (m)
        {
            glBindVertexArray(m->Resource.Id);
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
        glBindVertexArray(mesh->Resource.Id);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->ResVtx.Id);
        glBufferData(GL_ARRAY_BUFFER, mesh->VtxSize, mesh->Vtx, GL_STATIC_DRAW);

        GLint offset = 0;
        GLsizei stride = _GetAttribStride(mesh->Attribs);
        for (int i = 0; i < NyasVtxAttrib_COUNT; ++i)
        {
            if (!(mesh->Attribs & (1 << i)))
            {
                continue;
            }

            GLint size = attrib_sizes[i];
            GLint attrib_pos = glGetAttribLocation(shader_id, attrib_names[i]);
            if (attrib_pos >= 0)
            {
                glEnableVertexAttribArray(attrib_pos);
                glVertexAttribPointer(
                    attrib_pos, size, GL_FLOAT, GL_FALSE, stride, (void *)(offset * sizeof(float)));
            }
            offset += size;
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ResIdx.Id);
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

    void _NyCreateShader(uint32_t *id)
    {
        *id = glCreateProgram();
    }

    void _NyCompileShader(uint32_t id, const char *name, NyasShader *shader)
    {
        // For shader hot-recompilations
        GLuint shaders[8];
        GLsizei attach_count;
        glGetAttachedShaders(id, 8, &attach_count, shaders);
        for (int i = 0; i < attach_count; ++i)
        {
            glDetachShader(id, shaders[i]);
        }

        size_t shsrc_size; // Shader source size in bytes
        char *shsrc;
        char vert_path[256] = { '\0' };
        char frag_path[256] = { '\0' };
        strcpy(frag_path, "assets/shaders/");
        strcat(frag_path, name);
        strcpy(vert_path, frag_path);
        strcat(vert_path, "-vert.glsl");
        strcat(frag_path, "-frag.glsl");

        GLint err;
        GLchar output_log[1024];

        GLuint vert = glCreateShader(GL_VERTEX_SHADER);
        GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

        if (Nyas::ReadFile(vert_path, &shsrc, &shsrc_size) != NyasCode_Ok)
        {
            NYAS_ASSERT(!"Error loading shader vert file.");
        }

        glShaderSource(vert, 1, (const char *const *)&shsrc, NULL);
        glCompileShader(vert);
        glGetShaderiv(vert, GL_COMPILE_STATUS, &err);
        if (!err)
        {
            glGetShaderInfoLog(vert, 1024, NULL, output_log);
            NYAS_LOG_ERR("%s vert:\n%s\n", name, output_log);
        }
        NYAS_FREE(shsrc);

        if (Nyas::ReadFile(frag_path, &shsrc, &shsrc_size) != NyasCode_Ok)
        {
            NYAS_ASSERT(!"Error loading shader frag file.");
        }

        glShaderSource(frag, 1, (const char *const *)&shsrc, NULL);
        glCompileShader(frag);
        glGetShaderiv(frag, GL_COMPILE_STATUS, &err);
        if (!err)
        {
            glGetShaderInfoLog(frag, 1024, NULL, output_log);
            NYAS_LOG_ERR("%s frag:\n%s\n", name, output_log);
        }
        NYAS_FREE(shsrc);

        glAttachShader(id, vert);
        glAttachShader(id, frag);
        glLinkProgram(id);
        glGetProgramiv(id, GL_LINK_STATUS, &err);
        if (!err)
        {
            glGetProgramInfoLog(id, 1024, NULL, output_log);
            NYAS_LOG_ERR("%s program:\n%s\n", name, output_log);
        }

        glDeleteShader(vert);
        glDeleteShader(frag);

        if (!shader->UnitSize || !(shader->ResUnif.Flags & NyasResourceFlags_Unused))
        {
            glGenBuffers(1, &shader->ResUnif.Id);
            glBindBuffer(GL_UNIFORM_BUFFER, shader->ResUnif.Id);
            glBufferData(GL_UNIFORM_BUFFER, shader->UnitSize, shader->UnitBlock, GL_DYNAMIC_DRAW);
            glUniformBlockBinding(id, 30, 0);
        }

        if (!shader->SharedSize || !(shader->ResSharedUnif.Flags & NyasResourceFlags_Unused))
        {
            glGenBuffers(1, &shader->ResSharedUnif.Id);
            glBindBuffer(GL_UNIFORM_BUFFER, shader->ResSharedUnif.Id);
            glBufferData(GL_UNIFORM_BUFFER, shader->SharedSize, shader->SharedBlock, GL_DYNAMIC_DRAW);
            glUniformBlockBinding(id, 10, 0);
        }
    }

    void _NyShaderLocations(uint32_t id, int *o_loc, const char **i_unif, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            o_loc[i] = glGetUniformLocation(id, i_unif[i]);
        }
    }

    void _NySetShaderData(int loc, float *data, int v4count)
    {
        glUniform4fv(loc, v4count, data);
    }

    static void _SetTex(int loc, int *t, int c, int unit, GLenum target)
    {
        for (int i = 0; i < c; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + unit + i);
            glBindTexture(target, t[i]);
            t[i] = unit + i;
        }
        glUniform1iv(loc, c, t);
    }

    void _NySetShaderTex(int loc, int *tex, int count, int texunit_offset)
    {
        _SetTex(loc, tex, count, texunit_offset, GL_TEXTURE_2D);
    }

    void _NySetShaderCubemap(int loc, int *tex, int count, int texunit_offset)
    {
        _SetTex(loc, tex, count, texunit_offset, GL_TEXTURE_CUBE_MAP);
    }

    void _NySetShaderTexArray(int loc, int *tex, int count, int texunit_offset)
    {
        glUniform1iv(loc, count, tex);
        //_SetTex(loc, tex, count, texunit_offset, GL_TEXTURE_2D_ARRAY);
    }

    void _NySetShaderUniformBuffer(NyasShader* shader)
    {
        if (!shader->UnitSize || !(shader->ResUnif.Flags & NyasResourceFlags_Unused))
        {
            glBindBuffer(GL_UNIFORM_BUFFER, shader->ResUnif.Id);
            glBufferData(GL_UNIFORM_BUFFER, shader->UnitSize, shader->UnitBlock, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 30, shader->ResUnif.Id);
        }
        

        if (!shader->SharedSize || !(shader->ResSharedUnif.Flags & NyasResourceFlags_Unused))
        {
            glBindBuffer(GL_UNIFORM_BUFFER, shader->ResSharedUnif.Id);
            glBufferData(GL_UNIFORM_BUFFER, shader->SharedSize, shader->SharedBlock, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 10, shader->ResSharedUnif.Id);
        }
    }

    void _NyUseShader(uint32_t id)
    {
        glUseProgram(id);
    }

    void _NyReleaseShader(uint32_t id)
    {
        glDeleteProgram(id);
    }

    void _NyCreateFramebuf(NyasFramebuffer *fb)
    {
        glGenFramebuffers(1, &fb->Resource.Id);
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

    void _NySetFramebuf(uint32_t fb_id, uint32_t tex_id, NyasTexTarget *tt)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fb_id);
        GLenum face =
            tt->Face == NyasTexFace_2D ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP_POSITIVE_X + tt->Face;
        GLint slot = _GL_FramebufAttach(tt->Attach);
        glFramebufferTexture2D(GL_FRAMEBUFFER, slot, face, tex_id, tt->MipLevel);
    }

    void _NyUseFramebuf(uint32_t id)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void _NyReleaseFramebuf(NyasFramebuffer *fb)
    {
        glDeleteFramebuffers(1, &fb->Resource.Id);
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
        glDrawElementsInstanced(GL_TRIANGLES, elem_count, index_type ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0, instances);
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
}