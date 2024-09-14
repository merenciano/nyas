#include "nyas.h"

#include "nyas_math.h"
#include "nyas_render.h"
#include "nyas_types.h"

#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MEM_ALIGN 8
#define MEM_ALIGN_MOD(ADDRESS) ((ADDRESS) & (MEM_ALIGN - 1))

#define NYAS_MOUSE_BUTTON_UPDATE(MBTN)                                                             \
    G_Ctx->IO.MouseButton[(MBTN)] =                                                                \
        ((G_Ctx->IO.MouseButton[(MBTN)] << 1) |                                                    \
         glfwGetMouseButton((GLFWwindow *)G_Ctx->Platform.InternalWindow, (MBTN))) &               \
        3

#define NYAS_KEY_UPDATE(KEY)                                                                       \
    (G_Ctx->IO.Keys[(KEY)] =                                                                       \
         ((G_Ctx->IO.Keys[(KEY)] << 1) |                                                           \
          glfwGetKey((GLFWwindow *)G_Ctx->Platform.InternalWindow, (KEY))) &                       \
         3)

NyasCtx DefaultCtx;
NyasCtx *G_Ctx = &DefaultCtx;

namespace nyas {
NyPool<NyasFramebuffer> Framebufs;

NyPool<NyasEntity> Entities;
NyasCamera Camera;

static inline void _NyReadInput(void) {
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
    glfwGetCursorPos((GLFWwindow *)G_Ctx->Platform.InternalWindow, &x, &y);
    G_Ctx->IO.MousePosition = {(float)x, (float)y};
}

static void _NyScrollCallback(GLFWwindow *window, double x_offset, double y_offset) {
    NY_UNUSED(window);
    G_Ctx->IO.MouseScroll = {(float)x_offset, (float)y_offset};
}

static void _NyCursorEnterCallback(GLFWwindow *window, int entered) {
    NY_UNUSED(window);
    G_Ctx->Platform.WindowHovered = entered;
}

static void _NyWindowFocusCallback(GLFWwindow *window, int focused) {
    NY_UNUSED(window);
    G_Ctx->Platform.WindowFocused = focused;
}

NyasCtx *GetCurrentCtx() { return G_Ctx; }

bool InitIO(const char *title, int win_w, int win_h) {
    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    G_Ctx->Platform.InternalWindow = glfwCreateWindow(win_w, win_h, title, NULL, NULL);
    if (!G_Ctx->Platform.InternalWindow) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent((GLFWwindow *)G_Ctx->Platform.InternalWindow);
    nyas::render::_NySetProcLoader((void *(*)(const char *))glfwGetProcAddress);

    glfwSwapInterval(1);
    glfwSetScrollCallback((GLFWwindow *)G_Ctx->Platform.InternalWindow, _NyScrollCallback);
    glfwSetCursorEnterCallback((GLFWwindow *)G_Ctx->Platform.InternalWindow, _NyCursorEnterCallback);
    glfwSetWindowFocusCallback((GLFWwindow *)G_Ctx->Platform.InternalWindow, _NyWindowFocusCallback);
    glfwSetInputMode((GLFWwindow *)G_Ctx->Platform.InternalWindow, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(
        (GLFWwindow *)G_Ctx->Platform.InternalWindow, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    G_Ctx->Platform.WindowSize = {win_w, win_h};

    G_Ctx->Platform.ShowCursor = true;
    G_Ctx->Platform.CaptureMouse = true;
    G_Ctx->Platform.CaptureKeyboard = true;

    G_Ctx->Cfg.Navigation.Speed = 10.0f;
    G_Ctx->Cfg.Navigation.DragSensibility = 0.001f;
    G_Ctx->Cfg.Navigation.ScrollSensibility = 1.0f;

    nyas::render::_Init();

    PollIO();
    return true;
}

int ReadFile(const char *path, char **dst, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        NYAS_LOG_ERR("File open failed for %s.", path);
        return NyasError_File;
    }

    fseek(f, 0L, SEEK_END);
    *size = ftell(f) + 1;
    rewind(f);

    *dst = (char *)NYAS_ALLOC(*size);
    if (!*dst) {
        NYAS_LOG_ERR("Alloc (%lu bytes) failed.", *size);
        fclose(f);
        return NyasError_Alloc;
    }

    if (fread(*dst, *size - 1, 1, f) != 1) {
        NYAS_LOG_ERR("File read failed for %s.", path);
        NYAS_FREE(*dst);
        fclose(f);
        return NyasError_File;
    }

    fclose(f);
    (*dst)[*size - 1] = '\0';
    return NyasCode_Ok;
}

void PollIO() {
    NYAS_ASSERT(G_Ctx->Platform.InternalWindow && "The IO system is uninitalized");
    G_Ctx->IO.MouseScroll = {0.0f, 0.0f};
    glfwPollEvents();
    _NyReadInput();
    G_Ctx->Platform.WindowClosed =
        glfwWindowShouldClose((GLFWwindow *)G_Ctx->Platform.InternalWindow);
    glfwGetWindowSize(
        (GLFWwindow *)G_Ctx->Platform.InternalWindow, &G_Ctx->Platform.WindowSize.x,
        &G_Ctx->Platform.WindowSize.y);

    if (G_Ctx->Platform.ShowCursor) {
        glfwSetInputMode(
            (GLFWwindow *)G_Ctx->Platform.InternalWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(
            (GLFWwindow *)G_Ctx->Platform.InternalWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void WindowSwap() {
    NYAS_ASSERT(G_Ctx->Platform.InternalWindow && "The IO system is uninitalized");
    glfwSwapBuffers((GLFWwindow *)G_Ctx->Platform.InternalWindow);
}

template <typename T>
static inline void _NyCheckHandle(NyasHandle h, const NyPool<T> &pool) {
    NY_UNUSED(h), NY_UNUSED(pool);
    NYAS_ASSERT(h >= 0 && pool.Arr.Size > h && "Invalid handle range.");
}

using namespace nyas::render;

void Draw(NyasDrawCmd *cmd) {
    GTextures.Sync();
    GMeshes.Sync(GShaders.Pipelines[cmd->Pipeline].Attribs);

    if (cmd->Framebuf != NyasCode_NoOp) {
        GFb.Sync(cmd->Framebuf);
    }

    if (cmd->Pipeline != NyasCode_NoOp) {
        GShaders.Sync(cmd->Pipeline);
    }

    NyasDrawState &s = cmd->State;
    _NyViewport({s.ViewportMinX, s.ViewportMinY, s.ViewportMaxX, s.ViewportMaxY});
    _NyScissor({s.ScissorMinX, s.ScissorMinY, s.ScissorMaxX, s.ScissorMaxY});

    if (s.DisableFlags & NyasDrawFlags_DepthTest) {
        _NyDisableDepthTest();
    } else if (s.EnableFlags & NyasDrawFlags_DepthTest) {
        _NyEnableDepthTest();
    }

    if (s.DisableFlags & NyasDrawFlags_DepthWrite) {
        _NyDisableDepthMask();
    } else if (s.EnableFlags & NyasDrawFlags_DepthWrite) {
        _NyEnableDepthMask();
    }

    if (s.DisableFlags & NyasDrawFlags_StencilTest) {
        _NyDisableStencilTest();
    } else if (s.EnableFlags & NyasDrawFlags_StencilTest) {
        _NyEnableStencilTest();
    }

    if (s.DisableFlags & NyasDrawFlags_StencilWrite) {
        _NyDisableStencilMask();
    } else if (s.EnableFlags & NyasDrawFlags_StencilWrite) {
        _NyEnableStencilMask();
    }

    if (s.DisableFlags & NyasDrawFlags_Blend) {
        _NyDisableBlend();
    } else if (s.EnableFlags & NyasDrawFlags_Blend) {
        _NyEnableBlend();
    }

    if (s.DisableFlags & NyasDrawFlags_FaceCulling) {
        _NyDisableCull();
    } else if (s.EnableFlags & NyasDrawFlags_FaceCulling) {
        _NyEnableCull();
    }

    if (s.DisableFlags & NyasDrawFlags_Scissor) {
        _NyDisableScissor();
    } else if (s.EnableFlags & NyasDrawFlags_Scissor) {
        _NyEnableScissor();
    }

    _NySetDepthFunc(s.Depth);
    _NySetBlend(s.BlendSrc, s.BlendDst);
    _NySetCull(s.FaceCulling);

    _NyClearColor(s.BgColorR, s.BgColorG, s.BgColorB, s.BgColorA);
    _NyClear(s.EnableFlags & NyasDrawFlags_ColorClear, s.EnableFlags & NyasDrawFlags_DepthClear,
             s.EnableFlags & NyasDrawFlags_StencilClear);

    _NyDraw(cmd->Commands.data(), cmd->Commands.size());
}
} // namespace nyas

NyasHandle NYAS_SPHERE;
NyasHandle NYAS_CUBE;
NyasHandle NYAS_QUAD;

void NyasCamera::Navigate() {
    nyas::Vec3 eye = Eye();
    nyas::Vec3 fwd = nyas::Vec3::normal(Fwd()) * -1.0f;
    static nyas::Vec2 mouse_down_pos = {0.0f, 0.0f};
    float speed = G_Ctx->Cfg.Navigation.Speed * G_Ctx->Platform.DeltaTime;

    // Rotation
    if (G_Ctx->IO.MouseButton[NyasMouseButton_Right] == NyasKeyState_DOWN) {
        mouse_down_pos = G_Ctx->IO.MousePosition;
    }

    if (G_Ctx->IO.MouseButton[NyasMouseButton_Right] == NyasKeyState_PRESSED) {
        nyas::Vec2 curr_pos = G_Ctx->IO.MousePosition;
        nyas::Vec2 offset = {
            (curr_pos.x - mouse_down_pos.x) * G_Ctx->Cfg.Navigation.DragSensibility,
            (mouse_down_pos.y - curr_pos.y) * G_Ctx->Cfg.Navigation.DragSensibility};

        fwd +=
            (nyas::Vec3::cross(nyas::Vec3::up(), fwd) * -offset.x) + (nyas::Vec3::up() * offset.y);
        mouse_down_pos = curr_pos;
    }

    // Position
    if (G_Ctx->IO.Keys[NyasKey_W] == NyasKeyState_PRESSED) {
        eye += fwd * speed;
    }

    if (G_Ctx->IO.Keys[NyasKey_S] == NyasKeyState_PRESSED) {
        eye += fwd * -speed;
    }

    if (G_Ctx->IO.Keys[NyasKey_A] == NyasKeyState_PRESSED) {
        eye += nyas::Vec3::cross(nyas::Vec3::up(), fwd) * speed;
    }

    if (G_Ctx->IO.Keys[NyasKey_D] == NyasKeyState_PRESSED) {
        eye += nyas::Vec3::cross(nyas::Vec3::up(), fwd) * -speed;
    }

    if (G_Ctx->IO.Keys[NyasKey_Space] == NyasKeyState_PRESSED) {
        eye += (nyas::Vec3)nyas::Vec3::up() * speed;
    }

    if (G_Ctx->IO.Keys[NyasKey_LeftShift] == NyasKeyState_PRESSED) {
        eye += nyas::Vec3::up() * -speed;
    }

    mat4_look_at(View, eye, eye + fwd, nyas::Vec3::up());

    // Zoom
    if (G_Ctx->IO.MouseScroll.y != 0.0f) {
        Fov -= G_Ctx->IO.MouseScroll.y * G_Ctx->Cfg.Navigation.ScrollSensibility;
        Fov = nyas::clamp(Fov, 1.0f, 120.0f);
        Proj = nyas::Mat4::perspective(
            nyas::to_radians(Fov), G_Ctx->Platform.WindowSize.x,
            (float)G_Ctx->Platform.WindowSize.y, 0.1f, Far);
    }
}

static void _MeshSetCube(NyasHandle mesh) {
    static const float VERTICES[] = {
        -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.5f,  -0.5f, -0.5f,
        0.0f,  0.0f,  -1.0f, 1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
        1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  1.0f,

        -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, 0.5f,
        0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,

        -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f,
        -1.0f, 0.0f,  0.0f,  1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
        0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f,
        1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
        0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, -0.5f,
        0.0f,  -1.0f, 0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
        1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.0f,

        -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  -0.5f,
        0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
    };

    static const NyDrawIdx INDICES[] = {
        0,  2,  1,  2,  0,  3,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
        13, 12, 14, 12, 15, 14, 16, 17, 18, 18, 19, 16, 23, 22, 20, 22, 21, 20,
    };

    GMeshes.Update(
        mesh, (float *)VERTICES, sizeof(VERTICES) / sizeof(float), (NyDrawIdx *)INDICES,
        sizeof(INDICES) / sizeof(*INDICES));
}

static void _MeshSetSphere(NyasHandle mesh, int x_segments, int y_segments) {
    using namespace nyas;
    NYAS_ASSERT(y_segments > 2 && x_segments > 2 && "Invalid number of segments");

    const float x_step = 1.0f / (float)(y_segments - 1);
    const float y_step = 1.0f / (float)(x_segments - 1);

    int vtx_arr_count = y_segments * x_segments * 14;
    int idx_arr_count = y_segments * x_segments * 6;
    float *vtx_array = (float *)NYAS_ALLOC(vtx_arr_count * sizeof(float));
    NyDrawIdx *idx_array = (NyDrawIdx *)NYAS_ALLOC(idx_arr_count * sizeof(NyDrawIdx));

    float *v = vtx_array;
    constexpr Vec3 c = {0.0f, 0.0f, 0.0f};
    constexpr Vec3 up = {0.0f, 1.0f, 0.0f};
    for (int y = 0; y < x_segments; ++y) {
        Vec3 p;
        p.y = sinf((float)-M_PI_2 + (float)M_PI * (float)y * x_step);
        for (int x = 0; x < y_segments; ++x) {
            p.x = cosf(M_PI * 2.0f * x * y_step) * sinf(M_PI * y * x_step);
            p.z = sinf(M_PI * 2.0f * x * y_step) * sinf(M_PI * y * x_step);

            Vec3 tan = Vec3::normal(Vec3::cross(up, (p - c)));
            Vec3 bitan = Vec3::cross(p, tan);

            *v++ = p.x;
            *v++ = p.y;
            *v++ = p.z;
            *v++ = p.x;
            *v++ = p.y;
            *v++ = p.z;
            *v++ = tan.x;
            *v++ = tan.y;
            *v++ = tan.z;
            *v++ = bitan.x;
            *v++ = bitan.y;
            *v++ = bitan.z;

            *v++ = (float)x * y_step;
            *v++ = (float)y * x_step;
        }
    }

    NyDrawIdx *i = idx_array;
    for (int y = 0; y < x_segments; ++y) {
        for (int x = 0; x < y_segments; ++x) {
            *i++ = y * y_segments + x;
            *i++ = y * y_segments + x + 1;
            *i++ = (y + 1) * y_segments + x + 1;
            *i++ = y * y_segments + x;
            *i++ = (y + 1) * y_segments + x + 1;
            *i++ = (y + 1) * y_segments + x;
        }
    }

    GMeshes.Update(mesh, vtx_array, vtx_arr_count, idx_array, idx_arr_count);
}

static void _MeshSetQuad(NyasHandle mesh) {
    static const float VERTICES[] = {
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  1.0f, -1.0f, 0.0f,
        0.0f,  0.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  -1.0f,
        1.0f,  1.0f,  -1.0f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
    };

    static const NyDrawIdx INDICES[] = {0, 1, 2, 0, 2, 3};

    GMeshes.Update(
        mesh, (float *)VERTICES, sizeof(VERTICES) / sizeof(float), (NyDrawIdx *)INDICES,
        sizeof(INDICES) / sizeof(*INDICES));
}

namespace NyUtil {
static void _MeshSetGeometry(NyasHandle msh, NyasGeometry geo) {
    switch (geo) {
    case NyasGeometry_Quad:
        _MeshSetQuad(msh);
        break;
    case NyasGeometry_Cube:
        _MeshSetCube(msh);
        break;
    case NyasGeometry_Sphere:
        _MeshSetSphere(msh, 32, 32);
        break;
    default:
        break;
    }
}

void LoadBasicGeometries() {
    NyasVtxAttribFlags nmap_attr =
        NyasVtxAttribFlags_Normal | NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Tangent |
        NyasVtxAttribFlags_Bitangent | NyasVtxAttribFlags_UV;
    NYAS_SPHERE = GMeshes.Alloc(nmap_attr);
    NYAS_CUBE = GMeshes.Alloc(
        NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal | NyasVtxAttribFlags_UV);
    NYAS_QUAD = GMeshes.Alloc(
        NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal | NyasVtxAttribFlags_UV);
    _MeshSetGeometry(NYAS_SPHERE, NyasGeometry_Sphere);
    _MeshSetGeometry(NYAS_CUBE, NyasGeometry_Cube);
    _MeshSetGeometry(NYAS_QUAD, NyasGeometry_Quad);
}

void LoadEnv(
    const char *path, NyasTexture *lut, NyasTexture *sky, NyasTexture *irr, NyasTexture *pref) {
    FILE *f = fopen(path, "r");
    char hdr[9];
    fread(hdr, 8, 1, f);
    hdr[8] = '\0';
    if (strncmp("NYAS_ENV", hdr, 9) != 0) {
        NYAS_LOG_ERR("Header of .env file is invalid. Aborting load_env of %s.", path);
        return;
    }

    *sky = GTextures.Alloc({NyasTexFmt_RGB_16F, 1024, 1024, 1}, NyasTexFlags_Cubemap);
    size_t size = 1024 * 1024 * 3 * 2;
    NyasTexImage sky_img;
    sky_img.Level = 0;
    for (int i = 0; i < 6; ++i) {
        sky_img.Data[i] = NYAS_ALLOC(size);
        fread(sky_img.Data[i], size, 1, f);
        NYAS_ASSERT(sky_img.Data[i] && "The image couldn't be loaded");
    }
    GTextures.Update(*sky, sky_img);

    *irr = GTextures.Alloc({NyasTexFmt_RGB_16F, 1024, 1024, 1}, NyasTexFlags_Cubemap);
    NyasTexImage irrad_img;
    irrad_img.Level = 0;
    for (int i = 0; i < 6; ++i) {
        irrad_img.Data[i] = NYAS_ALLOC(size);
        fread(irrad_img.Data[i], size, 1, f);
        NYAS_ASSERT(irrad_img.Data[i] && "The image couldn't be loaded");
    }
    GTextures.Update(*irr, irrad_img);

    // TODO(OpenGL): Sampler - NyasTexFilter_LinearMipmapLinear
    *pref = GTextures.Alloc({NyasTexFmt_RGB_16F, 256, 256, 9}, NyasTexFlags_Cubemap);
    size = 256 * 256 * 3 * 2;
    for (int lod = 0; lod < 9; ++lod) {
        NyasTexImage img;
        img.Level = lod;
        for (int face = 0; face < 6; ++face) {
            img.Data[face] = NYAS_ALLOC(size);
            fread(img.Data[face], size, 1, f);
            NYAS_ASSERT(img.Data[face] && "The image couldn't be loaded");
        }
        GTextures.Update(*pref, img);
        size /= 4;
    }

    size = 512 * 512 * 2 * 2;
    void *img_pix{NYAS_ALLOC(size)};
    fread(img_pix, size, 1, f);
    *lut = GTextures.Alloc({NyasTexFmt_RG_16F, 512, 512, 1});
    GTextures.Update(*lut, {img_pix, 0});

    fclose(f);
}
} // namespace NyUtil
