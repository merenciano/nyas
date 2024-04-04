#include "nyas.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <mathc.h>
#include <pthread.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

#define MEM_ALIGN 8
#define MEM_ALIGN_MOD(ADDRESS) ((ADDRESS) & (MEM_ALIGN - 1))

#define NYAS_MOUSE_BUTTON_UPDATE(MBTN)                                                             \
    G_Ctx->IO.MouseButton[(MBTN)] =                                                                \
        ((G_Ctx->IO.MouseButton[(MBTN)] << 1) |                                                    \
            glfwGetMouseButton((GLFWwindow *)G_Ctx->IO.InternalWindow, (MBTN))) &                  \
        3

#define NYAS_KEY_UPDATE(KEY)                                                                       \
    (G_Ctx->IO.Keys[(KEY)] = ((G_Ctx->IO.Keys[(KEY)] << 1) |                                       \
                                 glfwGetKey((GLFWwindow *)G_Ctx->IO.InternalWindow, (KEY))) &      \
                             3)

typedef int NyasSchedState;
enum NyasSchedState_
{
    NyasSchedState_None,
    NyasSchedState_Running,
    NyasSchedState_Closing,
    NyasSchedState_Closed,
    NyasSchedState_COUNT
};

struct _NyScheduler
{
    NyArray<NySched::Job> Queue;
    NyArray<pthread_t> Threads;
    pthread_mutex_t Mtx;
    pthread_cond_t Cond;
    int Waiting;
    NyasSchedState State;

    _NyScheduler() : Waiting(0), State(NyasSchedState_None) {}
};

// ---
// [PRIVATE]
// ---
void _NyCreateTex(NyasTexture *t);
void _NySetTex(NyasTexture *t);
void _NyReleaseTex(uint32_t *id);

void _NyCreateMesh(uint32_t *id, uint32_t *vid, uint32_t *iid);
void _NyUseMesh(NyasMesh *m, NyasShader *s);
void _NySetMesh(NyasMesh *mesh, uint32_t shader_id);
void _NyReleaseMesh(uint32_t *id, uint32_t *vid, uint32_t *iid);

void _NyCreateShader(uint32_t *id);
void _NyCompileShader(uint32_t id, const char *name);
void _NyUseShader(uint32_t id);
void _NyReleaseShader(uint32_t id);
void _NyShaderLocations(uint32_t id, int *o_loc, const char **i_unif, int count);
void _NySetShaderData(int loc, float *data, int v4count);
void _NySetShaderTex(int loc, int *tex, int count, int texunit_offset);
void _NySetShaderCubemap(int loc, int *tex, int count, int texunit_offset);

void _NyCreateFramebuf(NyasFramebuffer *fb);
void _NySetFramebuf(uint32_t fb_id, uint32_t tex_id, NyasTexTarget *tt);
void _NyUseFramebuf(uint32_t id);
void _NyReleaseFramebuf(NyasFramebuffer *fb);

void _NyClear(bool color = true, bool depth = true, bool stencil = false);
void _NyDraw(int elem_count, int index_type);
void _NyClearColor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
void _NyEnableScissor();
void _NyDisableScissor();
void _NyEnableBlend();
void _NyDisableBlend();
void _NySetBlend(NyasBlendFunc blend_func_src, NyasBlendFunc blend_func_dst);
void _NyEnableCull();
void _NyDisableCull();
void _NySetCull(NyasFaceCull cull_face);
void _NyEnableDepthTest();
void _NyDisableDepthTest();
void _NyEnableDepthMask();
void _NyDisableDepthMask();
void _NySetDepthFunc(NyasDepthFunc depth_func);
void _NyEnableStencilTest();
void _NyDisableStencilTest();
void _NyEnableStencilMask();
void _NyDisableStencilMask();
void _NyViewport(NyRect rect);
void _NyScissor(NyRect rect);

NyasCtx DefaultCtx;
NyasCtx *G_Ctx = &DefaultCtx;

namespace Nyas
{
NyPool<NyasMesh> Meshes;
NyPool<NyasTexture> Textures;
NyPool<NyasShader> Shaders;
NyPool<NyasFramebuffer> Framebufs;

NyPool<NyasEntity> Entities;
NyasCamera Camera;

static inline void ReadInput(void)
{
    NYAS_KEY_UPDATE(NyasKey_Invalid);
    NYAS_KEY_UPDATE(NyasKey_Space);
    NYAS_KEY_UPDATE(NyasKey_Apostrophe);
    NYAS_KEY_UPDATE(NyasKey_Comma);
    NYAS_KEY_UPDATE(NyasKey_Minus);
    NYAS_KEY_UPDATE(NyasKey_Period);
    NYAS_KEY_UPDATE(NyasKey_Slash);
    NYAS_KEY_UPDATE(NyasKey_0);
    NYAS_KEY_UPDATE(NyasKey_1);
    NYAS_KEY_UPDATE(NyasKey_2);
    NYAS_KEY_UPDATE(NyasKey_3);
    NYAS_KEY_UPDATE(NyasKey_4);
    NYAS_KEY_UPDATE(NyasKey_5);
    NYAS_KEY_UPDATE(NyasKey_6);
    NYAS_KEY_UPDATE(NyasKey_7);
    NYAS_KEY_UPDATE(NyasKey_8);
    NYAS_KEY_UPDATE(NyasKey_9);
    NYAS_KEY_UPDATE(NyasKey_Semicolon);
    NYAS_KEY_UPDATE(NyasKey_Equal);
    NYAS_KEY_UPDATE(NyasKey_A);
    NYAS_KEY_UPDATE(NyasKey_B);
    NYAS_KEY_UPDATE(NyasKey_C);
    NYAS_KEY_UPDATE(NyasKey_D);
    NYAS_KEY_UPDATE(NyasKey_E);
    NYAS_KEY_UPDATE(NyasKey_F);
    NYAS_KEY_UPDATE(NyasKey_G);
    NYAS_KEY_UPDATE(NyasKey_H);
    NYAS_KEY_UPDATE(NyasKey_I);
    NYAS_KEY_UPDATE(NyasKey_J);
    NYAS_KEY_UPDATE(NyasKey_K);
    NYAS_KEY_UPDATE(NyasKey_L);
    NYAS_KEY_UPDATE(NyasKey_M);
    NYAS_KEY_UPDATE(NyasKey_N);
    NYAS_KEY_UPDATE(NyasKey_O);
    NYAS_KEY_UPDATE(NyasKey_P);
    NYAS_KEY_UPDATE(NyasKey_Q);
    NYAS_KEY_UPDATE(NyasKey_R);
    NYAS_KEY_UPDATE(NyasKey_S);
    NYAS_KEY_UPDATE(NyasKey_T);
    NYAS_KEY_UPDATE(NyasKey_U);
    NYAS_KEY_UPDATE(NyasKey_V);
    NYAS_KEY_UPDATE(NyasKey_W);
    NYAS_KEY_UPDATE(NyasKey_X);
    NYAS_KEY_UPDATE(NyasKey_Y);
    NYAS_KEY_UPDATE(NyasKey_Z);
    NYAS_KEY_UPDATE(NyasKey_LeftBracket);
    NYAS_KEY_UPDATE(NyasKey_Backslash);
    NYAS_KEY_UPDATE(NyasKey_RightBracket);
    NYAS_KEY_UPDATE(NyasKey_GraveAccent);
    NYAS_KEY_UPDATE(NyasKey_World1);
    NYAS_KEY_UPDATE(NyasKey_World2);
    NYAS_KEY_UPDATE(NyasKey_Escape);
    NYAS_KEY_UPDATE(NyasKey_Enter);
    NYAS_KEY_UPDATE(NyasKey_Tab);
    NYAS_KEY_UPDATE(NyasKey_Backspace);
    NYAS_KEY_UPDATE(NyasKey_Insert);
    NYAS_KEY_UPDATE(NyasKey_Delete);
    NYAS_KEY_UPDATE(NyasKey_Right);
    NYAS_KEY_UPDATE(NyasKey_Left);
    NYAS_KEY_UPDATE(NyasKey_Down);
    NYAS_KEY_UPDATE(NyasKey_Up);
    NYAS_KEY_UPDATE(NyasKey_PageUp);
    NYAS_KEY_UPDATE(NyasKey_PageDown);
    NYAS_KEY_UPDATE(NyasKey_Home);
    NYAS_KEY_UPDATE(NyasKey_End);
    NYAS_KEY_UPDATE(NyasKey_CapsLock);
    NYAS_KEY_UPDATE(NyasKey_ScrollLock);
    NYAS_KEY_UPDATE(NyasKey_NumLock);
    NYAS_KEY_UPDATE(NyasKey_PrintScreen);
    NYAS_KEY_UPDATE(NyasKey_Pause);
    NYAS_KEY_UPDATE(NyasKey_F1);
    NYAS_KEY_UPDATE(NyasKey_F2);
    NYAS_KEY_UPDATE(NyasKey_F3);
    NYAS_KEY_UPDATE(NyasKey_F4);
    NYAS_KEY_UPDATE(NyasKey_F5);
    NYAS_KEY_UPDATE(NyasKey_F6);
    NYAS_KEY_UPDATE(NyasKey_F7);
    NYAS_KEY_UPDATE(NyasKey_F8);
    NYAS_KEY_UPDATE(NyasKey_F9);
    NYAS_KEY_UPDATE(NyasKey_F10);
    NYAS_KEY_UPDATE(NyasKey_F11);
    NYAS_KEY_UPDATE(NyasKey_F12);
    NYAS_KEY_UPDATE(NyasKey_F13);
    NYAS_KEY_UPDATE(NyasKey_F14);
    NYAS_KEY_UPDATE(NyasKey_F15);
    NYAS_KEY_UPDATE(NyasKey_F16);
    NYAS_KEY_UPDATE(NyasKey_F17);
    NYAS_KEY_UPDATE(NyasKey_F18);
    NYAS_KEY_UPDATE(NyasKey_F19);
    NYAS_KEY_UPDATE(NyasKey_F20);
    NYAS_KEY_UPDATE(NyasKey_F21);
    NYAS_KEY_UPDATE(NyasKey_F22);
    NYAS_KEY_UPDATE(NyasKey_F23);
    NYAS_KEY_UPDATE(NyasKey_F24);
    NYAS_KEY_UPDATE(NyasKey_F25);
    NYAS_KEY_UPDATE(NyasKey_Kp0);
    NYAS_KEY_UPDATE(NyasKey_Kp1);
    NYAS_KEY_UPDATE(NyasKey_Kp2);
    NYAS_KEY_UPDATE(NyasKey_Kp3);
    NYAS_KEY_UPDATE(NyasKey_Kp4);
    NYAS_KEY_UPDATE(NyasKey_Kp5);
    NYAS_KEY_UPDATE(NyasKey_Kp6);
    NYAS_KEY_UPDATE(NyasKey_Kp7);
    NYAS_KEY_UPDATE(NyasKey_Kp8);
    NYAS_KEY_UPDATE(NyasKey_Kp9);
    NYAS_KEY_UPDATE(NyasKey_KpDecimal);
    NYAS_KEY_UPDATE(NyasKey_KpDivide);
    NYAS_KEY_UPDATE(NyasKey_KpMultiply);
    NYAS_KEY_UPDATE(NyasKey_KpSubtract);
    NYAS_KEY_UPDATE(NyasKey_KpAdd);
    NYAS_KEY_UPDATE(NyasKey_KpEnter);
    NYAS_KEY_UPDATE(NyasKey_KpEqual);
    NYAS_KEY_UPDATE(NyasKey_LeftShift);
    NYAS_KEY_UPDATE(NyasKey_LeftControl);
    NYAS_KEY_UPDATE(NyasKey_LeftAlt);
    NYAS_KEY_UPDATE(NyasKey_LeftSuper);
    NYAS_KEY_UPDATE(NyasKey_RightShift);
    NYAS_KEY_UPDATE(NyasKey_RightControl);
    NYAS_KEY_UPDATE(NyasKey_RightAlt);
    NYAS_KEY_UPDATE(NyasKey_RightSuper);
    NYAS_KEY_UPDATE(NyasKey_Menu);

    NYAS_MOUSE_BUTTON_UPDATE(NyasMouseButton_Left);
    NYAS_MOUSE_BUTTON_UPDATE(NyasMouseButton_Right);
    NYAS_MOUSE_BUTTON_UPDATE(NyasMouseButton_Middle);

    double x, y;
    glfwGetCursorPos((GLFWwindow *)G_Ctx->IO.InternalWindow, &x, &y);
    G_Ctx->IO.MousePosition = { (float)x, (float)y };
}

static void nyas__scrollcallback(GLFWwindow *window, double x_offset, double y_offset)
{
    NY_UNUSED(window);
    G_Ctx->IO.MouseScroll = { (float)x_offset, (float)y_offset };
}

static void nyas__cursor_enter_callback(GLFWwindow *window, int entered)
{
    NY_UNUSED(window);
    G_Ctx->Platform.WindowHovered = entered;
}

static void nyas__window_focus_callback(GLFWwindow *window, int focused)
{
    NY_UNUSED(window);
    G_Ctx->Platform.WindowFocused = focused;
}

NyasCtx *GetCurrentCtx()
{
    return G_Ctx;
}

bool InitIO(const char *title, int win_w, int win_h)
{
    memset(&G_Ctx->IO, 0, sizeof(G_Ctx->IO));
    if (!glfwInit())
    {
        return false;
    }

    G_Ctx->IO.InternalWindow = glfwCreateWindow(win_w, win_h, title, NULL, NULL);
    if (!G_Ctx->IO.InternalWindow)
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent((GLFWwindow *)G_Ctx->IO.InternalWindow);
#ifndef __EMSCRIPTEN__
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif

    glfwSwapInterval(1);
    glfwSetScrollCallback((GLFWwindow *)G_Ctx->IO.InternalWindow, nyas__scrollcallback);
    glfwSetCursorEnterCallback((GLFWwindow *)G_Ctx->IO.InternalWindow, nyas__cursor_enter_callback);
    glfwSetWindowFocusCallback((GLFWwindow *)G_Ctx->IO.InternalWindow, nyas__window_focus_callback);
    glfwSetInputMode((GLFWwindow *)G_Ctx->IO.InternalWindow, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode((GLFWwindow *)G_Ctx->IO.InternalWindow, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    G_Ctx->IO.ShowCursor = true;
    G_Ctx->IO.CaptureMouse = true;
    G_Ctx->IO.CaptureKeyboard = true;

    G_Ctx->Cfg.Navigation.Speed = 10.0f;
    G_Ctx->Cfg.Navigation.DragSensibility = 0.001f;
    G_Ctx->Cfg.Navigation.ScrollSensibility = 1.0f;

    PollIO();
    return true;
}

int nyas_file_read(const char *path, char **dst, size_t *size)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        NYAS_LOG_ERR("File open failed for %s.", path);
        return NyasError_File;
    }

    fseek(f, 0L, SEEK_END);
    *size = ftell(f) + 1;
    rewind(f);

    *dst = (char *)NYAS_ALLOC(*size);
    if (!*dst)
    {
        NYAS_LOG_ERR("Alloc (%lu bytes) failed.", *size);
        fclose(f);
        return NyasError_Alloc;
    }

    if (fread(*dst, *size - 1, 1, f) != 1)
    {
        NYAS_LOG_ERR("File read failed for %s.", path);
        NYAS_FREE(*dst);
        fclose(f);
        return NyasError_File;
    }

    fclose(f);
    (*dst)[*size - 1] = '\0';
    return NyasCode_Ok;
}

void PollIO(void)
{
    NYAS_ASSERT(G_Ctx->IO.InternalWindow && "The IO system is uninitalized");
    G_Ctx->IO.MouseScroll = { 0.0f, 0.0f };
    glfwPollEvents();
    ReadInput();
    G_Ctx->Platform.WindowClosed = glfwWindowShouldClose((GLFWwindow *)G_Ctx->IO.InternalWindow);
    glfwGetWindowSize(
        (GLFWwindow *)G_Ctx->IO.InternalWindow, &G_Ctx->IO.WindowSize.X, &G_Ctx->IO.WindowSize.Y);

    if (G_Ctx->IO.ShowCursor)
    {
        glfwSetInputMode((GLFWwindow *)G_Ctx->IO.InternalWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        glfwSetInputMode((GLFWwindow *)G_Ctx->IO.InternalWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void WindowSwap(void)
{
    NYAS_ASSERT(G_Ctx->IO.InternalWindow && "The IO system is uninitalized");
    glfwSwapBuffers((GLFWwindow *)G_Ctx->IO.InternalWindow);
}

template<typename T> static inline void _NyCheckHandle(NyasHandle h, const NyPool<T> &pool)
{
    NY_UNUSED(h), NY_UNUSED(pool);
    NYAS_ASSERT(h >= 0 && pool.Arr.Size > h && "Invalid handle range.");
}

static void
nyas__file_reader(void *_1, const char *path, int _2, const char *_3, char **buf, size_t *size)
{
    (void)_1, (void)_2, (void)_3;
    nyas_file_read(path, buf, size);
}

static NyasHandle _CreateMeshHandle(void)
{
    return Meshes.Add(NyasMesh());
}

static NyasHandle _CreateFramebufHandle(void)
{
    return Framebufs.Add(NyasFramebuffer());
}

static NyasHandle _CreateShaderHandle(void)
{
    return Shaders.Add(NyasShader());
}

const char *_GetImgFacePath(const char *path, int face, int face_count)
{
    switch (face_count)
    {
        case 6:
        case 5:
        case 4:
        case 3:
        case 2:
        {
            const char *suffixes = "RLUDFB";
            static char buffer[1024];
            int count = snprintf(buffer, 1024, path, suffixes[face]);
            if (count < 1024)
            {
                NYAS_LOG_ERR("Cubemap face path format: %s is too long!", path);
                return NULL;
            }
            return buffer;
        }
        case 1: return path;
        default: NYAS_LOG_WARN("Invalid cubemap face count. Path is %s.", path); return path;
    }
}

static int _TexChannels(NyasTexFmt fmt)
{
    switch (fmt)
    {
        case NyasTexFmt_RGBA_16F:
        case NyasTexFmt_RGBA_8: return 4;
        case NyasTexFmt_RGB_16F:
        case NyasTexFmt_RGB_8:
        case NyasTexFmt_SRGB_8: return 3;
        case NyasTexFmt_RG_16F:
        case NyasTexFmt_RG_8: return 2;
        case NyasTexFmt_R_16F:
        case NyasTexFmt_R_8: return 1;
        default: return 0;
    }
}

static int _TexFaces(NyasTexType type)
{
    switch (type)
    {
        case NyasTexType_2D:
        case NyasTexType_Array2D: return 1;
        case NyasTexType_Cubemap:
        case NyasTexType_ArrayCubemap: return 6;
        default: return 0;
    }
}

static bool _TexFmtFloat(NyasTexFmt fmt)
{
    switch (fmt)
    {
        case NyasTexFmt_RGBA_16F:
        case NyasTexFmt_RGB_16F:
        case NyasTexFmt_RGB_32F:
        case NyasTexFmt_RG_16F:
        case NyasTexFmt_R_16F: return true;
        default: return false;
    }
}

NyasHandle CreateTexture(void)
{
    int tex = Textures.Add(NyasTexture());
    return tex;
}

void LoadTexture(NyasHandle texture, struct NyasTexDesc *desc, const char *path)
{
    NYAS_ASSERT(*path != '\0' && "For empty textures use nyas_tex_set");
    NyasTexture *t = &Textures[texture];
    t->Resource.Id = 0;
    t->Resource.Flags = NyasResourceFlags_Dirty;
    t->Data = *desc;

    int fmt_ch = _TexChannels(t->Data.Format);
    stbi_set_flip_vertically_on_load(t->Data.Flags & NyasTexFlags_FlipVerticallyOnLoad);

    int channels = 0;
    int face_count = _TexFaces(t->Data.Type);
    for (int i = 0; i < face_count; ++i)
    {
        NyasTexImg img;
        const char *p = _GetImgFacePath(path, i, face_count);
        img.Face = i;
        if (_TexFmtFloat(t->Data.Format))
        {
            img.Pix = stbi_loadf(p, &t->Data.Width, &t->Data.Height, &channels, fmt_ch);
        }
        else
        {
            img.Pix = stbi_load(p, &t->Data.Width, &t->Data.Height, &channels, fmt_ch);
        }

        if (!img.Pix)
        {
            NYAS_LOG_ERR("The image '%s' couldn't be loaded", p);
        }
        t->Img.Push(img);
    }
}

void SetTexture(NyasHandle texture, struct NyasTexDesc *desc)
{
    NYAS_ASSERT(desc->Width > 0 && desc->Height > 0 && "Incorrect dimensions");
    NyasTexture *t = &Textures[texture];
    t->Resource.Flags |= NyasResourceFlags_Dirty;
    t->Data = *desc;
    if (!t->Img.Size)
    {
        int face_count = _TexFaces(t->Data.Type);
        for (int i = 0; i < face_count; ++i)
        {
            NyasTexImg img;
            img.Face = i;
            t->Img.Push(img);
        }
    }
}

NyasHandle CreateShader(const struct NyasShaderDesc *desc)
{
    NyasHandle ret = _CreateShaderHandle();
    Shaders[ret].Name = desc->Name;
    Shaders[ret].Resource.Id = 0;
    Shaders[ret].Resource.Flags = NyasResourceFlags_Dirty;
    Shaders[ret].Count[0].Data = desc->DataCount;
    Shaders[ret].Count[0].Tex = desc->TexCount;
    Shaders[ret].Count[0].Cubemap = desc->CubemapCount;
    Shaders[ret].Count[1].Data = desc->SharedDataCount;
    Shaders[ret].Count[1].Tex = desc->SharedTexCount;
    Shaders[ret].Count[1].Cubemap = desc->SharedCubemapCount;
    Shaders[ret].Shared = NYAS_ALLOC(
        (desc->SharedDataCount + desc->SharedTexCount + desc->SharedCubemapCount) * sizeof(float));
    return ret;
}

void *GetMaterialSharedData(NyasHandle shader)
{
    return Shaders[shader].Shared;
}

NyasHandle *GetMaterialSharedTextures(NyasHandle shader)
{
    NyasShader *shdr = &Shaders[shader];
    return (NyasHandle *)shdr->Shared + shdr->Count[1].Data;
}

NyasHandle *GetMaterialSharedCubemaps(NyasHandle shader)
{
    NyasShader *shdr = &Shaders[shader];
    return GetMaterialSharedTextures(shader) + shdr->Count[1].Tex;
}

void ReloadShader(NyasHandle shader)
{
    Shaders[shader].Resource.Flags |= NyasResourceFlags_Dirty;
}

static NyDrawIdx _CheckVertex(const float *v, const float *end, const float *newvtx)
{
    NyDrawIdx i = 0;
    for (; v < end; ++i, v += 14)
    {
        if ((v[0] == newvtx[0]) && (v[1] == newvtx[1]) && (v[2] == newvtx[2]) &&
            (v[3] == newvtx[3]) && (v[4] == newvtx[4]) && (v[5] == newvtx[5]) &&
            (v[12] == newvtx[12]) && (v[13] == newvtx[13]))
        {
            return i;
        }
    }
    return i;
}

static void _SetMeshObj(NyasMesh *mesh, const char *path)
{
    tinyobj_attrib_t attrib;
    tinyobj_shape_t *shapes = NULL;
    size_t shape_count;
    tinyobj_material_t *mats = NULL;
    size_t mats_count;

    int result = tinyobj_parse_obj(&attrib, &shapes, &shape_count, &mats, &mats_count, path,
        nyas__file_reader, NULL, TINYOBJ_FLAG_TRIANGULATE);

    NYAS_ASSERT(result == TINYOBJ_SUCCESS && "Obj loader failed.");
    if (result != TINYOBJ_SUCCESS)
    {
        NYAS_LOG_ERR("Error loading obj. Err: %d", result);
    }

    size_t vertex_count = attrib.num_face_num_verts * 3;

    NYAS_FREE(mesh->Vtx);
    NYAS_FREE(mesh->Indices);

    mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal |
                    NyasVtxAttribFlags_Tangent | NyasVtxAttribFlags_Bitangent |
                    NyasVtxAttribFlags_UV;
    mesh->VtxSize = vertex_count * 14 * sizeof(float);
    mesh->ElementCount = vertex_count;
    mesh->Vtx = (float *)NYAS_ALLOC(mesh->VtxSize);
    mesh->Indices = (NyDrawIdx *)NYAS_ALLOC(mesh->ElementCount * sizeof(NyDrawIdx));

    float *vit = mesh->Vtx;

    size_t index_offset = 0;
    for (size_t i = 0; i < attrib.num_face_num_verts; ++i)
    {
        for (int f = 0; f < attrib.face_num_verts[i] / 3; ++f)
        {
            tinyobj_vertex_index_t idx = attrib.faces[3 * f + index_offset];
            float v1[14], v2[14], v3[14];

            v1[0] = attrib.vertices[3 * idx.v_idx + 0];
            v1[1] = attrib.vertices[3 * idx.v_idx + 1];
            v1[2] = attrib.vertices[3 * idx.v_idx + 2];
            v1[3] = attrib.normals[3 * idx.vn_idx + 0];
            v1[4] = attrib.normals[3 * idx.vn_idx + 1];
            v1[5] = attrib.normals[3 * idx.vn_idx + 2];
            v1[12] = attrib.texcoords[2 * idx.vt_idx + 0];
            v1[13] = attrib.texcoords[2 * idx.vt_idx + 1];

            idx = attrib.faces[3 * f + index_offset + 1];
            v2[0] = attrib.vertices[3 * idx.v_idx + 0];
            v2[1] = attrib.vertices[3 * idx.v_idx + 1];
            v2[2] = attrib.vertices[3 * idx.v_idx + 2];
            v2[3] = attrib.normals[3 * idx.vn_idx + 0];
            v2[4] = attrib.normals[3 * idx.vn_idx + 1];
            v2[5] = attrib.normals[3 * idx.vn_idx + 2];
            v2[12] = attrib.texcoords[2 * idx.vt_idx + 0];
            v2[13] = attrib.texcoords[2 * idx.vt_idx + 1];

            idx = attrib.faces[3 * f + index_offset + 2];
            v3[0] = attrib.vertices[3 * idx.v_idx + 0];
            v3[1] = attrib.vertices[3 * idx.v_idx + 1];
            v3[2] = attrib.vertices[3 * idx.v_idx + 2];
            v3[3] = attrib.normals[3 * idx.vn_idx + 0];
            v3[4] = attrib.normals[3 * idx.vn_idx + 1];
            v3[5] = attrib.normals[3 * idx.vn_idx + 2];
            v3[12] = attrib.texcoords[2 * idx.vt_idx + 0];
            v3[13] = attrib.texcoords[2 * idx.vt_idx + 1];

            // Calculate tangent and bitangent
            float delta_p1[3], delta_p2[3], delta_uv1[2], delta_uv2[2];
            vec3_subtract(delta_p1, v2, v1);
            vec3_subtract(delta_p2, v3, v1);
            vec2_subtract(delta_uv1, &v2[12], &v1[12]);
            vec2_subtract(delta_uv2, &v3[12], &v1[12]);
            float r = 1.0f / (delta_uv1[0] * delta_uv2[1] - delta_uv1[1] * delta_uv2[0]);

            float tn[3], bitn[3], tmp[3];
            vec3_multiply_f(tn, delta_p1, delta_uv2[1]);
            vec3_multiply_f(tmp, delta_p2, delta_uv1[1]);
            vec3_multiply_f(tn, vec3_subtract(tn, tn, tmp), r);

            vec3_multiply_f(bitn, delta_p2, delta_uv1[0]);
            vec3_multiply_f(tmp, delta_p1, delta_uv2[0]);
            vec3_multiply_f(bitn, vec3_subtract(bitn, bitn, tmp), r);

            v1[6] = tn[0];
            v1[7] = tn[1];
            v1[8] = tn[2];
            v2[6] = tn[0];
            v2[7] = tn[1];
            v2[8] = tn[2];
            v3[6] = tn[0];
            v3[7] = tn[1];
            v3[8] = tn[2];

            v1[9] = bitn[0];
            v1[10] = bitn[1];
            v1[11] = bitn[2];
            v2[9] = bitn[0];
            v2[10] = bitn[1];
            v2[11] = bitn[2];
            v3[9] = bitn[0];
            v3[10] = bitn[1];
            v3[11] = bitn[2];

            // Check vertex rep
            NyDrawIdx nxt_idx = _CheckVertex(mesh->Vtx, vit, v1);
            mesh->Indices[index_offset++] = nxt_idx;
            if (nxt_idx * 14 == (vit - mesh->Vtx))
            {
                for (int j = 0; j < 14; ++j)
                {
                    *vit++ = v1[j];
                }
            }

            nxt_idx = _CheckVertex(mesh->Vtx, vit, v2);
            mesh->Indices[index_offset++] = nxt_idx;
            if (nxt_idx * 14 == (vit - mesh->Vtx))
            {
                for (int j = 0; j < 14; ++j)
                {
                    *vit++ = v2[j];
                }
            }

            nxt_idx = _CheckVertex(mesh->Vtx, vit, v3);
            mesh->Indices[index_offset++] = nxt_idx;
            if (nxt_idx * 14 == (vit - mesh->Vtx))
            {
                for (int j = 0; j < 14; ++j)
                {
                    *vit++ = v3[j];
                }
            }
        }
    }

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, shape_count);
    tinyobj_materials_free(mats, mats_count);
}

static void _SetMeshMsh(NyasMesh *mesh, const char *path)
{
    char *data;
    size_t sz;
    nyas__file_reader(NULL, path, 0, NULL, &data, &sz);
    if (!data || !sz)
    {
        NYAS_LOG_ERR("Problem reading file %s", path);
        return;
    }

    NYAS_FREE(mesh->Vtx);
    NYAS_FREE(mesh->Indices);

    mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal |
                    NyasVtxAttribFlags_Tangent | NyasVtxAttribFlags_Bitangent |
                    NyasVtxAttribFlags_UV;
    mesh->VtxSize = *(size_t *)data;
    data += sizeof(size_t);
    mesh->Vtx = (float *)NYAS_ALLOC(mesh->VtxSize);
    memcpy(mesh->Vtx, data, mesh->VtxSize);
    data += mesh->VtxSize;

    mesh->ElementCount = (*(size_t *)data) / sizeof(NyDrawIdx);
    data += sizeof(size_t);
    mesh->Indices = (NyDrawIdx *)NYAS_ALLOC(mesh->ElementCount * sizeof(NyDrawIdx));
    memcpy(mesh->Indices, data, mesh->ElementCount * sizeof(NyDrawIdx));

    NYAS_FREE(data - mesh->VtxSize - (2 * sizeof(size_t)));
}

void ReloadMesh(NyasHandle msh, const char *path)
{
    NyasMesh *m = &Meshes[msh];
    size_t len = strlen(path);
    const char *extension = path + len;
    while (*--extension != '.')
    {
    }
    extension++;
    if (!strcmp(extension, "obj"))
    {
        _SetMeshObj(m, path);
    }
    else if (!strcmp(extension, "msh"))
    {
        _SetMeshMsh(m, path);
    }
    else
    {
        NYAS_LOG_ERR("Extension (%s) of file %s not recognised.", extension, path);
    }

    m->Resource.Flags |= NyasResourceFlags_Dirty;
}

static NyasHandle _NewMesh(void)
{
    NyasHandle mesh_handle = _CreateMeshHandle();
    Meshes[mesh_handle].Resource.Id = 0;
    Meshes[mesh_handle].Resource.Flags = NyasResourceFlags_Dirty;
    Meshes[mesh_handle].Attribs = 0;
    Meshes[mesh_handle].Vtx = NULL;
    Meshes[mesh_handle].Indices = NULL;
    Meshes[mesh_handle].VtxSize = 0;
    Meshes[mesh_handle].ElementCount = 0;
    Meshes[mesh_handle].ResVtx.Id = 0;
    Meshes[mesh_handle].ResVtx.Flags = NyasResourceFlags_Dirty;
    Meshes[mesh_handle].ResIdx.Id = 0;
    Meshes[mesh_handle].ResIdx.Flags = NyasResourceFlags_Dirty;

    return mesh_handle;
}

NyasHandle CreateMesh(void)
{
    return _NewMesh();
}

NyasHandle LoadMesh(const char *path)
{
    NyasHandle mesh_handle = _NewMesh();
    ReloadMesh(mesh_handle, path);
    return mesh_handle;
}

NyasHandle CreateFramebuffer(void)
{
    NyasHandle framebuffer = _CreateFramebufHandle();
    Framebufs[framebuffer].Resource.Id = 0;
    Framebufs[framebuffer].Resource.Flags = NyasResourceFlags_Dirty;
    for (int i = 0; i < 8; ++i)
    {
        Framebufs[framebuffer].Target[i].Tex = NyasCode_None;
    }
    return framebuffer;
}

void SetFramebufferTarget(NyasHandle framebuffer, int index, struct NyasTexTarget target)
{
    Framebufs[framebuffer].Resource.Flags |= NyasResourceFlags_Dirty;
    Framebufs[framebuffer].Target[index] = target;
}

NyasMaterial CreateMaterial(NyasHandle shader)
{
    NyasMaterial ret(shader);
    NyasShader *s = &Shaders[shader];
    int elements = s->Count[0].Data + s->Count[0].Tex + s->Count[0].Cubemap;
    ret.Ptr = NYAS_ALLOC(elements * sizeof(float));
    return ret;
}

NyasMaterial CreateMaterialTmp(NyasHandle shader)
{
    NyasMaterial ret(shader);
    NyasShader *s = &Shaders[shader];
    int elements = s->Count[0].Data + s->Count[0].Tex + s->Count[0].Cubemap;
    ret.Ptr = NyFrameAllocator::Alloc(elements * sizeof(float));
    return ret;
}

NyasMaterial CopyMaterialTmp(NyasMaterial mat)
{
    NyasMaterial ret(mat.Shader);
    NyasShader *s = &Shaders[mat.Shader];
    size_t size = (s->Count[0].Data + s->Count[0].Tex + s->Count[0].Cubemap) * 4;
    ret.Ptr = NyFrameAllocator::Alloc(size);
    memcpy(ret.Ptr, mat.Ptr, size);
    return ret;
}

NyasMaterial CopyShaderMaterialTmp(NyasHandle shader)
{
    NyasMaterial ret(shader);
    NyasShader *s = &Shaders[shader];
    int elements = s->Count[1].Data + s->Count[1].Tex + s->Count[1].Cubemap;
    ret.Ptr = NyFrameAllocator::Alloc(elements * sizeof(float));
    memcpy(ret.Ptr, s->Shared, elements * sizeof(float));
    return ret;
}

NyasHandle *GetMaterialTextures(NyasMaterial mat)
{
    return (NyasHandle *)mat.Ptr + Shaders[mat.Shader].Count[0].Data;
}

static void _SyncMesh(NyasHandle msh, NyasHandle shader)
{
    _NyCheckHandle(msh, Meshes);
    _NyCheckHandle(shader, Shaders);
    NyasMesh *m = &Meshes[msh];

    if (!(m->Resource.Flags & NyasResourceFlags_Created))
    {
        _NyCreateMesh(&m->Resource.Id, &m->ResVtx.Id, &m->ResIdx.Id);
        m->Resource.Flags |= NyasResourceFlags_Created;
    }

    if (m->Resource.Flags & NyasResourceFlags_Dirty)
    {
        _NySetMesh(m, Shaders[shader].Resource.Id);
        m->Resource.Flags &= ~NyasResourceFlags_Dirty;
    }
}

static NyasTexture *_SyncTex(NyasHandle texture)
{
    _NyCheckHandle(texture, Textures);
    NyasTexture *t = &Textures[texture];
    if (!(t->Resource.Flags & NyasResourceFlags_Created))
    {
        _NyCreateTex(t);
        t->Resource.Flags |= (NyasResourceFlags_Created | NyasResourceFlags_Dirty);
    }

    if (t->Resource.Flags & NyasResourceFlags_Dirty)
    {
        _NySetTex(t);
        t->Resource.Flags &= ~NyasResourceFlags_Dirty;
    }
    return t;
}

static void _SyncShaderData(NyasShader *s, void *srcdata, int common)
{
    NYAS_ASSERT((common == 0 || common == 1) && "Invalid common value.");

    int dc = s->Count[common].Data;
    int tc = s->Count[common].Tex;
    int cc = s->Count[common].Cubemap;
    int dl = s->Location[common].Data;
    int tl = s->Location[common].Tex;
    int cl = s->Location[common].Cubemap;
    int tex_unit = NYAS_TEXUNIT_OFFSET_FOR_COMMON_SHADER_DATA * common;

    NYAS_ASSERT((dc >= 0) && (tc >= 0) && (cc >= 0));
    NYAS_ASSERT(tex_unit >= 0 && tex_unit < 128);

    if (!(dc + tc + cc))
    {
        return;
    }

    // change texture handle for texture internal id
    NyasHandle *data_tex = (NyasHandle *)srcdata + dc;
    for (int i = 0; i < tc + cc; ++i)
    {
        NyasTexture *itx = _SyncTex(data_tex[i]);
        data_tex[i] = (int)itx->Resource.Id;
    }

    // set opengl uniforms
    if (dc)
    {
        _NySetShaderData(dl, (float *)srcdata, dc / 4);
    }

    if (tc)
    {
        _NySetShaderTex(tl, data_tex, tc, tex_unit);
    }

    if (cc)
    {
        _NySetShaderCubemap(cl, data_tex + tc, cc, tex_unit + tc);
    }
}

void _SyncShader(NyasShader *s)
{
    static const char *uniforms[] = { "u_data", "u_tex", "u_cube", "u_shared_data", "u_common_tex",
        "u_common_cube" };

    if (!(s->Resource.Flags & NyasResourceFlags_Created))
    {
        _NyCreateShader(&s->Resource.Id);
        s->Resource.Flags |= NyasResourceFlags_Created;
    }

    if (s->Resource.Flags & NyasResourceFlags_Dirty)
    {
        NYAS_ASSERT(s->Name && *s->Name && "Shader name needed.");
        _NyCompileShader(s->Resource.Id, s->Name);
        _NyShaderLocations(s->Resource.Id, &s->Location[0].Data, &uniforms[0], 6);
        s->Resource.Flags &= ~NyasResourceFlags_Dirty;
    }
}

static void _SyncFramebuf(NyasHandle framebuffer)
{
    // nypx__check_handle(framebuffer, &Framebufs);
    struct NyasFramebuffer *fb = &Framebufs[framebuffer];
    if (!(fb->Resource.Flags & NyasResourceFlags_Created))
    {
        _NyCreateFramebuf(fb);
        fb->Resource.Flags |= NyasResourceFlags_Created;
    }

    _NyUseFramebuf(fb->Resource.Id);
    if (fb->Resource.Flags & NyasResourceFlags_Dirty)
    {
        for (int i = 0; fb->Target[i].Tex != NyasCode_None; ++i)
        {
            struct NyasTexture *t = _SyncTex(fb->Target[i].Tex);
            _NySetFramebuf(fb->Resource.Id, t->Resource.Id, &fb->Target[i]);
        }
        fb->Resource.Flags &= ~NyasResourceFlags_Dirty;
    }
}

void Draw(NyasDrawCmd *cmd)
{
    if (cmd->Framebuf != NyasCode_NoOp)
    {
        if (cmd->Framebuf == NyasCode_Default)
        {
            _NyUseFramebuf(0);
        }
        else
        {
            // nypx__check_handle(cmd->fb, &Framebufs);
            _SyncFramebuf(cmd->Framebuf);
        }
    }
    //_draw_target(cmd->fb);

    if (cmd->ShaderMaterial.Shader != NyasCode_NoOp)
    {
        // nypx__check_handle(mat.shader, &shader_pool);
        _NyCheckHandle(cmd->ShaderMaterial.Shader, Shaders);
        NyasShader *s = &Shaders[cmd->ShaderMaterial.Shader];
        _SyncShader(s);
        _NyUseShader(s->Resource.Id);
        _SyncShaderData(s, cmd->ShaderMaterial.Ptr, true);
    }

    NyasDrawState &s = cmd->State;
    _NyClearColor(s.BgColorR, s.BgColorG, s.BgColorB, s.BgColorA);
    _NyClear(s.EnableFlags & NyasDrawFlags_ColorClear, s.EnableFlags & NyasDrawFlags_DepthClear,
        s.EnableFlags & NyasDrawFlags_StencilClear);

    _NyViewport({ s.ViewportMinX, s.ViewportMinY, s.ViewportMaxX, s.ViewportMaxY });
    _NyScissor({ s.ScissorMinX, s.ScissorMinY, s.ScissorMaxX, s.ScissorMaxY });

    if (s.DisableFlags & NyasDrawFlags_DepthTest)
    {
        _NyDisableDepthTest();
    }
    else if (s.EnableFlags & NyasDrawFlags_DepthTest)
    {
        _NyEnableDepthTest();
    }

    if (s.DisableFlags & NyasDrawFlags_DepthWrite)
    {
        _NyDisableDepthMask();
    }
    else if (s.EnableFlags & NyasDrawFlags_DepthWrite)
    {
        _NyEnableDepthMask();
    }

    if (s.DisableFlags & NyasDrawFlags_StencilTest)
    {
        _NyDisableStencilTest();
    }
    else if (s.EnableFlags & NyasDrawFlags_StencilTest)
    {
        _NyEnableStencilTest();
    }

    if (s.DisableFlags & NyasDrawFlags_StencilWrite)
    {
        _NyDisableStencilMask();
    }
    else if (s.EnableFlags & NyasDrawFlags_StencilWrite)
    {
        _NyEnableStencilMask();
    }

    if (s.DisableFlags & NyasDrawFlags_Blend)
    {
        _NyDisableBlend();
    }
    else if (s.EnableFlags & NyasDrawFlags_Blend)
    {
        _NyEnableBlend();
    }

    if (s.DisableFlags & NyasDrawFlags_FaceCulling)
    {
        _NyDisableCull();
    }
    else if (s.EnableFlags & NyasDrawFlags_FaceCulling)
    {
        _NyEnableCull();
    }

    if (s.DisableFlags & NyasDrawFlags_Scissor)
    {
        _NyDisableScissor();
    }
    else if (s.EnableFlags & NyasDrawFlags_Scissor)
    {
        _NyEnableScissor();
    }

    _NySetDepthFunc(s.Depth);
    _NySetBlend(s.BlendSrc, s.BlendDst);
    _NySetCull(s.FaceCulling);

    for (int i = 0; i < cmd->UnitCount; ++i)
    {
        NyasMesh *imsh = &Meshes[cmd->Units[i].Mesh];
        _NyCheckHandle(cmd->Units[i].Mesh, Meshes);
        NYAS_ASSERT(imsh->ElementCount && "Attempt to draw an uninitialized mesh");

        if (imsh->Resource.Flags & NyasResourceFlags_Dirty)
        {
            _SyncMesh(cmd->Units[i].Mesh, cmd->Units[i].Material.Shader);
        }

        NyasShader *s = &Shaders[cmd->Units[i].Material.Shader];
        _SyncShaderData(s, cmd->Units[i].Material.Ptr, false);
        _NyUseMesh(imsh, s);
        _NyDraw(imsh->ElementCount, sizeof(NyDrawIdx) == 4);
    }
}
} // namespace Nyas

NyasHandle NYAS_SPHERE;
NyasHandle NYAS_CUBE;
NyasHandle NYAS_QUAD;

void NyasCamera::Navigate()
{
    NyVec3 eye = Eye();
    NyVec3 fwd = Fwd();
    vec3_negative(fwd, vec3_normalize(fwd, fwd));
    static NyVec2 mouse_down_pos;
    float speed = G_Ctx->Cfg.Navigation.Speed * G_Ctx->Platform.DeltaTime;

    // Rotation
    if (G_Ctx->IO.MouseButton[NyasMouseButton_Right] == NyasKeyState_DOWN)
    {
        mouse_down_pos = G_Ctx->IO.MousePosition;
    }

    float tmp_vec[3];
    if (G_Ctx->IO.MouseButton[NyasMouseButton_Right] == NyasKeyState_PRESSED)
    {
        NyVec2 curr_pos = G_Ctx->IO.MousePosition;
        NyVec2 offset = { (curr_pos.X - mouse_down_pos.X) * G_Ctx->Cfg.Navigation.DragSensibility,
            (mouse_down_pos.Y - curr_pos.Y) * G_Ctx->Cfg.Navigation.DragSensibility };

        vec3_add(
            fwd, fwd, vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, NyVec3::Up(), fwd), -offset.X));
        vec3_add(fwd, fwd, vec3_multiply_f(tmp_vec, NyVec3::Up(), offset.Y));

        mouse_down_pos = curr_pos;
    }

    // Position
    if (G_Ctx->IO.Keys[NyasKey_W] == NyasKeyState_PRESSED)
    {
        vec3_add(eye, eye, vec3_multiply_f(tmp_vec, fwd, speed));
    }

    if (G_Ctx->IO.Keys[NyasKey_S] == NyasKeyState_PRESSED)
    {
        vec3_add(eye, eye, vec3_multiply_f(tmp_vec, fwd, -speed));
    }

    if (G_Ctx->IO.Keys[NyasKey_A] == NyasKeyState_PRESSED)
    {
        vec3_add(eye, eye, vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, NyVec3::Up(), fwd), speed));
    }

    if (G_Ctx->IO.Keys[NyasKey_D] == NyasKeyState_PRESSED)
    {
        vec3_add(
            eye, eye, vec3_multiply_f(tmp_vec, vec3_cross(tmp_vec, NyVec3::Up(), fwd), -speed));
    }

    if (G_Ctx->IO.Keys[NyasKey_Space] == NyasKeyState_PRESSED)
    {
        vec3_add(eye, eye, vec3_multiply_f(tmp_vec, NyVec3::Up(), speed));
    }

    if (G_Ctx->IO.Keys[NyasKey_LeftShift] == NyasKeyState_PRESSED)
    {
        vec3_add(eye, eye, vec3_multiply_f(tmp_vec, NyVec3::Up(), -speed));
    }

    mat4_look_at(View, eye, vec3_add(tmp_vec, eye, fwd), NyVec3::Up());

    // Zoom
    if (G_Ctx->IO.MouseScroll.Y != 0.0f)
    {
        Fov -= G_Ctx->IO.MouseScroll.Y * G_Ctx->Cfg.Navigation.ScrollSensibility;
        Fov = clampf(Fov, 1.0f, 120.0f);
        mat4_perspective(Proj, to_radians(Fov),
            (float)G_Ctx->IO.WindowSize.X / (float)G_Ctx->IO.WindowSize.Y, 0.1f, Far);
    }
}

static void *_Worker(void *data)
{
    _NyScheduler *s = (_NyScheduler *)data;
    while (1)
    {
        ++s->Waiting;
        pthread_mutex_lock(&s->Mtx);
        while (!(s->Queue.Size))
        {
            pthread_cond_wait(&s->Cond, &s->Mtx);
            if (s->State == NyasSchedState_Closing)
            {
                --s->Waiting;
                pthread_mutex_unlock(&s->Mtx);
                goto exit_worker;
            }
        }

        --s->Waiting;
        NySched::Job job = s->Queue.Back();
        s->Queue.Pop();
        pthread_mutex_unlock(&s->Mtx);
        (*(job.Func))(job.Args);
    }

exit_worker:
    return NULL;
}

NySched::NySched(int thread_count)
{
    _Sched = (_NyScheduler *)NYAS_ALLOC(sizeof(_NyScheduler));
    *_Sched = _NyScheduler();

    pthread_mutex_init(&_Sched->Mtx, NULL);
    pthread_cond_init(&_Sched->Cond, NULL);

    for (int i = 0; i < thread_count; ++i)
    {
        pthread_t thread;
        if (pthread_create(&thread, NULL, _Worker, _Sched))
        {
            NYAS_LOG_ERR("Thread creation error.");
            return;
        }
        _Sched->Threads.Push(thread);
    }

    _Sched->State = NyasSchedState_Running;
}

NySched::~NySched()
{
    pthread_mutex_lock(&_Sched->Mtx);
    _Sched->State = NyasSchedState_Closing;
    pthread_cond_broadcast(&_Sched->Cond);
    pthread_mutex_unlock(&_Sched->Mtx);
    for (int i = 0; i < _Sched->Threads.Size; ++i)
    {
        pthread_join(_Sched->Threads[i], NULL);
    }

    pthread_mutex_lock(&_Sched->Mtx);
    pthread_mutex_unlock(&_Sched->Mtx);
    pthread_mutex_destroy(&_Sched->Mtx);
    pthread_cond_destroy(&_Sched->Cond);

    _Sched->~_NyScheduler();
    NYAS_FREE(_Sched);
}

void NySched::Do(Job job)
{
    if (!_Sched->Threads.Size)
    {
        (*(job.Func))(job.Args);
        return;
    }
    pthread_mutex_lock(&_Sched->Mtx);
    _Sched->Queue.Push(job);
    pthread_cond_signal(&_Sched->Cond);
    pthread_mutex_unlock(&_Sched->Mtx);
}

void NySched::Wait()
{
    if (!_Sched->Threads.Size)
    {
        return;
    }

    while (_Sched->Queue.Size || _Sched->Waiting != _Sched->Threads.Size)
    {
        pthread_cond_broadcast(&_Sched->Cond);
        const timespec sleep_time({ 0, 50000000L });
        nanosleep(&sleep_time, NULL);
    }
}

static void _TexLoader(void *arg)
{
    NyAssetLoader::TexArgs *a = (NyAssetLoader::TexArgs *)arg;
    Nyas::LoadTexture(a->Tex, &a->Descriptor, a->Path);
}

static void _MeshLoader(void *arg)
{
    NyAssetLoader::MeshArgs *a = (NyAssetLoader::MeshArgs *)arg;
    *a->Mesh = Nyas::LoadMesh(a->Path); // TODO: separate create and load like textures in order to
                                        // avoid concurrent writes in mesh_pool
}

static void _ShaderLoader(void *arg)
{
    NyAssetLoader::ShaderArgs *a = (NyAssetLoader::ShaderArgs *)arg;
    *a->Shader = Nyas::CreateShader(&a->Descriptor);
    // TODO: Shaders compilation and program linking.
}

static void _EnvLoader(void *args)
{
    NyAssetLoader::EnvArgs *ea = (NyAssetLoader::EnvArgs *)args;
    NyUtil::LoadEnv(ea->Path, ea->LUT, ea->Sky, ea->Irradiance, ea->Pref);
}

void NyAssetLoader::AddMesh(MeshArgs *args)
{
    Async.Push({ _MeshLoader, args });
}

void NyAssetLoader::AddTex(TexArgs *args)
{
    Async.Push({ _TexLoader, args });
}

void NyAssetLoader::AddShader(ShaderArgs *args)
{
    Sequential.Push({ _ShaderLoader, args });
}

void NyAssetLoader::AddEnv(EnvArgs *args)
{
    Async.Push({ _EnvLoader, args });
}

void NyAssetLoader::AddJob(NySched::Job job, bool async)
{
    if (async)
    {
        Async.Push(job);
    }
    else
    {
        Sequential.Push(job);
    }
}

void NyAssetLoader::Load(int threads)
{
    NySched load_sched = NySched(threads);
    for (int i = 0; i < Async.Size; ++i)
    {
        load_sched.Do(Async[i]);
    }

    for (int i = 0; i < Sequential.Size; ++i)
    {
        (*Sequential[i].Func)(Sequential[i].Args);
    }

    load_sched.Wait(); // TODO(Check): sched_destroy waits?
}

static void _MeshSetCube(NyasMesh *mesh)
{
    static const float VERTICES[] = {
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        -1.0f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        -0.5f,
        0.0f,
        0.0f,
        -1.0f,
        1.0f,
        1.0f,
        -0.5f,
        0.5f,
        -0.5f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f,

        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.5f,
        -0.5f,
        0.5f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        0.5f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.5f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,

        -0.5f,
        0.5f,
        0.5f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        -0.5f,
        0.5f,
        -0.5f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        -0.5f,
        -0.5f,
        -0.5f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        -0.5f,
        -0.5f,
        0.5f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,

        0.5f,
        0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.5f,
        -0.5f,
        -0.5f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        0.5f,
        -0.5f,
        0.5f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        0.0f,

        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.5f,
        -0.5f,
        0.5f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,

        -0.5f,
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.5f,
        0.5f,
        -0.5f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.5f,
        0.5f,
        0.5f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.0f,
        -0.5f,
        0.5f,
        0.5f,
        0.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
    };

    static const NyDrawIdx INDICES[] = {
        0,
        2,
        1,
        2,
        0,
        3,
        4,
        5,
        6,
        6,
        7,
        4,
        8,
        9,
        10,
        10,
        11,
        8,
        13,
        12,
        14,
        12,
        15,
        14,
        16,
        17,
        18,
        18,
        19,
        16,
        23,
        22,
        20,
        22,
        21,
        20,
    };

    NYAS_FREE(mesh->Vtx);
    NYAS_FREE(mesh->Indices);

    mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal | NyasVtxAttribFlags_UV;
    mesh->Vtx = (float *)NYAS_ALLOC(sizeof(VERTICES));
    memcpy(mesh->Vtx, VERTICES, sizeof(VERTICES));
    mesh->Indices = (NyDrawIdx *)NYAS_ALLOC(sizeof(INDICES));
    memcpy(mesh->Indices, INDICES, sizeof(INDICES));
    mesh->VtxSize = sizeof(VERTICES);
    mesh->ElementCount = sizeof(INDICES) / sizeof(*INDICES);
}

static void _MeshSetSphere(NyasMesh *mesh, int x_segments, int y_segments)
{
    NYAS_ASSERT(y_segments > 2 && x_segments > 2 && "Invalid number of segments");

    const float x_step = 1.0f / (float)(y_segments - 1);
    const float y_step = 1.0f / (float)(x_segments - 1);

    NYAS_FREE(mesh->Vtx);
    NYAS_FREE(mesh->Indices);

    mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal | NyasVtxAttribFlags_UV;
    mesh->VtxSize = y_segments * x_segments * 8 * sizeof(float);
    mesh->ElementCount = y_segments * x_segments * 6;
    mesh->Vtx = (float *)NYAS_ALLOC(mesh->VtxSize);
    mesh->Indices = (NyDrawIdx *)NYAS_ALLOC(mesh->ElementCount * sizeof(NyDrawIdx));

    float *v = mesh->Vtx;
    for (int y = 0; y < x_segments; ++y)
    {
        float py = sinf((float)-M_PI_2 + (float)M_PI * (float)y * x_step);
        for (int x = 0; x < y_segments; ++x)
        {
            float px = cosf(M_PI * 2.0f * x * y_step) * sinf(M_PI * y * x_step);
            float pz = sinf(M_PI * 2.0f * x * y_step) * sinf(M_PI * y * x_step);

            *v++ = px;
            *v++ = py;
            *v++ = pz;
            *v++ = px;
            *v++ = py;
            *v++ = pz;
            *v++ = (float)x * y_step;
            *v++ = (float)y * x_step;
        }
    }

    NyDrawIdx *i = mesh->Indices;
    for (int y = 0; y < x_segments; ++y)
    {
        for (int x = 0; x < y_segments; ++x)
        {
            *i++ = y * y_segments + x;
            *i++ = y * y_segments + x + 1;
            *i++ = (y + 1) * y_segments + x + 1;
            *i++ = y * y_segments + x;
            *i++ = (y + 1) * y_segments + x + 1;
            *i++ = (y + 1) * y_segments + x;
        }
    }
}

static void _MeshSetQuad(NyasMesh *mesh)
{
    static const float VERTICES[] = {
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        0.0f,
        1.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        -1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        0.0f,
        0.0f,
        0.0f,
        -1.0f,
        0.0f,
        1.0f,
    };

    static const NyDrawIdx INDICES[] = { 0, 1, 2, 0, 2, 3 };

    NYAS_FREE(mesh->Vtx);
    NYAS_FREE(mesh->Indices);

    mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal | NyasVtxAttribFlags_UV;
    mesh->Vtx = (float *)NYAS_ALLOC(sizeof(VERTICES));
    memcpy(mesh->Vtx, VERTICES, sizeof(VERTICES));
    mesh->Indices = (NyDrawIdx *)NYAS_ALLOC(sizeof(INDICES));
    memcpy(mesh->Indices, INDICES, sizeof(INDICES));
    mesh->VtxSize = sizeof(VERTICES);
    mesh->ElementCount = sizeof(INDICES) / sizeof(*INDICES);
}

namespace NyUtil
{
static void _MeshSetGeometry(NyasHandle msh, NyasGeometry geo)
{
    NyasMesh *m = &Nyas::Meshes[msh];

    switch (geo)
    {
        case NyasGeometry_Quad: _MeshSetQuad(m); break;
        case NyasGeometry_Cube: _MeshSetCube(m); break;
        case NyasGeometry_Sphere: _MeshSetSphere(m, 32, 32); break;
        default: break;
    }
    m->Resource.Flags |= NyasResourceFlags_Dirty;
}

void LoadBasicGeometries(void)
{
    NYAS_SPHERE = Nyas::CreateMesh();
    NYAS_CUBE = Nyas::CreateMesh();
    NYAS_QUAD = Nyas::CreateMesh();
    _MeshSetGeometry(NYAS_SPHERE, NyasGeometry_Sphere);
    _MeshSetGeometry(NYAS_CUBE, NyasGeometry_Cube);
    _MeshSetGeometry(NYAS_QUAD, NyasGeometry_Quad);
}

void LoadEnv(const char *path, NyasHandle *lut, NyasHandle *sky, NyasHandle *irr, NyasHandle *pref)
{
    FILE *f = fopen(path, "r");
    char hdr[9];
    fread(hdr, 8, 1, f);
    hdr[8] = '\0';
    if (strncmp("NYAS_ENV", hdr, 9) != 0)
    {
        NYAS_LOG_ERR("Header of .env file is invalid. Aborting load_env of %s.", path);
        return;
    }

    *sky = Nyas::CreateTexture();
    NyasTexture *t = &Nyas::Textures[*sky];
    t->Resource.Id = 0;
    t->Resource.Flags = NyasResourceFlags_Dirty;
    t->Data.Type = NyasTexType_Cubemap;
    t->Data.Format = NyasTexFmt_RGB_16F;
    t->Data.Width = 1024;
    t->Data.Height = 1024;
    t->Data.MagFilter = NyasTexFilter_Linear;
    t->Data.MinFilter = NyasTexFilter_Linear;
    t->Data.WrapS = NyasTexWrap_Clamp;
    t->Data.WrapT = NyasTexWrap_Clamp;
    t->Data.WrapR = NyasTexWrap_Clamp;

    size_t size = 1024 * 1024 * 3 * 2; // size * nchannels * sizeof(channel)
    for (int i = 0; i < 6; ++i)
    {
        NyasTexImg img;
        img.Face = i;
        img.Pix = NYAS_ALLOC(size);
        fread(img.Pix, size, 1, f);
        NYAS_ASSERT(img.Pix && "The image couldn't be loaded");
        t->Img.Push(img);
    }

    *irr = Nyas::CreateTexture();
    t = &Nyas::Textures[*irr];
    t->Resource.Id = 0;
    t->Resource.Flags = NyasResourceFlags_Dirty;
    t->Data.Type = NyasTexType_Cubemap;
    t->Data.Format = NyasTexFmt_RGB_16F;
    t->Data.Width = 1024;
    t->Data.Height = 1024;
    t->Data.MagFilter = NyasTexFilter_Linear;
    t->Data.MinFilter = NyasTexFilter_Linear;
    t->Data.WrapS = NyasTexWrap_Clamp;
    t->Data.WrapT = NyasTexWrap_Clamp;
    t->Data.WrapR = NyasTexWrap_Clamp;

    for (int i = 0; i < 6; ++i)
    {
        NyasTexImg img;
        img.Face = i;
        img.Pix = NYAS_ALLOC(size);
        fread(img.Pix, size, 1, f);
        NYAS_ASSERT(img.Pix && "The image couldn't be loaded");
        t->Img.Push(img);
    }

    *pref = Nyas::CreateTexture();
    t = &Nyas::Textures[*pref];
    t->Resource.Id = 0;
    t->Resource.Flags = NyasResourceFlags_Dirty;
    t->Data.Type = NyasTexType_Cubemap;
    t->Data.Format = NyasTexFmt_RGB_16F;
    t->Data.Width = 256;
    t->Data.Height = 256;
    t->Data.MagFilter = NyasTexFilter_Linear;
    t->Data.MinFilter = NyasTexFilter_LinearMipmapLinear;
    t->Data.WrapS = NyasTexWrap_Clamp;
    t->Data.WrapT = NyasTexWrap_Clamp;
    t->Data.WrapR = NyasTexWrap_Clamp;

    size = 256 * 256 * 3 * 2;
    for (int lod = 0; lod < 9; ++lod)
    {
        for (int face = 0; face < 6; ++face)
        {
            NyasTexImg img;
            img.MipLevel = lod;
            img.Face = face;
            img.Pix = NYAS_ALLOC(size);
            fread(img.Pix, size, 1, f);
            NYAS_ASSERT(img.Pix && "The image couldn't be loaded");
            t->Img.Push(img);
        }
        size /= 4;
    }

    *lut = Nyas::CreateTexture();
    t = &Nyas::Textures[*lut];
    t->Resource.Id = 0;
    t->Resource.Flags = NyasResourceFlags_Dirty;
    t->Data.Type = NyasTexType_2D;
    t->Data.Format = NyasTexFmt_RG_16F;
    t->Data.Width = 512;
    t->Data.Height = 512;
    t->Data.MagFilter = NyasTexFilter_Linear;
    t->Data.MinFilter = NyasTexFilter_Linear;
    t->Data.WrapS = NyasTexWrap_Clamp;
    t->Data.WrapT = NyasTexWrap_Clamp;
    t->Data.WrapR = NyasTexWrap_Clamp;

    size = 512 * 512 * 2 * 2;

    NyasTexImg img;
    img.Pix = NYAS_ALLOC(size);
    fread(img.Pix, size, 1, f);
    NYAS_ASSERT(img.Pix && "The image couldn't be loaded");
    t->Img.Push(img);
    fclose(f);
}
} // namespace NyUtil

#if defined(NYAS_GL3)

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

static NyGL_TexDesc _GL_TexDesc(NyasTexDesc *d)
{
    return { _GL_TexTarget(d->Type), _GL_TexFilter(d->MinFilter), _GL_TexFilter(d->MagFilter),
        _GL_TexWrap(d->WrapS), _GL_TexWrap(d->WrapT), _GL_TexWrap(d->WrapR), d->BorderColor[0],
        d->BorderColor[1], d->BorderColor[2], d->BorderColor[3] };
}

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

void _NyCreateTex(struct NyasTexture *t)
{
    NyGL_TexDesc d = _GL_TexDesc(&t->Data);
    glGenTextures(1, &t->Resource.Id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(d.target, t->Resource.Id);
    if (d.min_f)
    {
        glTexParameteri(d.target, GL_TEXTURE_MIN_FILTER, d.min_f);
    }
    if (d.mag_f)
    {
        glTexParameteri(d.target, GL_TEXTURE_MAG_FILTER, d.mag_f);
    }
    if (d.ws)
    {
        glTexParameteri(d.target, GL_TEXTURE_WRAP_S, d.ws);
    }
    if (d.wt)
    {
        glTexParameteri(d.target, GL_TEXTURE_WRAP_T, d.wt);
    }
    if (d.wr)
    {
        glTexParameteri(d.target, GL_TEXTURE_WRAP_R, d.wr);
    }
    if (d.ba > 0.0f)
    {
        glTexParameterfv(d.target, GL_TEXTURE_BORDER_COLOR, &d.br);
    }
}

void _NySetTex(struct NyasTexture *t)
{
    GLenum type = _GL_TexTarget(t->Data.Type);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(type, t->Resource.Id);
    struct _GL_TexFmtResult fmt = _GL_TexFmt(t->Data.Format);
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

void _NyCompileShader(uint32_t id, const char *name)
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

    if (Nyas::nyas_file_read(vert_path, &shsrc, &shsrc_size) != NyasCode_Ok)
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

    if (Nyas::nyas_file_read(frag_path, &shsrc, &shsrc_size) != NyasCode_Ok)
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

void _NyUseShader(uint32_t id)
{
    glUseProgram(id);
}

void _NyReleaseShader(uint32_t id)
{
    glDeleteProgram(id);
}

void _NyCreateFramebuf(struct NyasFramebuffer *fb)
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

void _NySetFramebuf(uint32_t fb_id, uint32_t tex_id, struct NyasTexTarget *tt)
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

void _NyReleaseFramebuf(struct NyasFramebuffer *fb)
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

void _NyDraw(int elem_count, int index_type)
{
    glDrawElements(GL_TRIANGLES, elem_count, index_type ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, 0);
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

#endif