#ifndef NYAS_H
#define NYAS_H

#ifdef NYAS_USER_CONFIG_H
#include NYAS_USER_CONFIG_H
#endif
#include "nyconfig.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <mathc.h>

#ifndef NYAS_ASSERT
#include <assert.h>
#define NYAS_ASSERT(_COND) assert(_COND)
#endif

#if !defined(NYAS_ALLOC) || !defined(NYAS_FREE)
#include <stdlib.h>
#undef NYAS_ALLOC
#undef NYAS_FREE
#define NYAS_ALLOC(_SIZE) malloc(_SIZE)
#define NYAS_FREE(_PTR) free(_PTR)
#endif

#ifndef NYAS_GET_TIME_NS
#include <chrono>
#define NYAS_GET_TIME_NS (std::chrono::system_clock::now().time_since_epoch().count())
#endif

#define NY_UNUSED(_VAR) (void)(_VAR)
#define NY_KILOBYTES(_BYTES) ((_BYTES) * 1024L)
#define NY_MEGABYTES(_BYTES) (NY_KILOBYTES(_BYTES) * 1024L)
#define NY_GIGABYTES(_BYTES) (NYAS_MB(_BYTES) * 1024L)

#ifndef NYAS_PRINT
#define NYAS_PRINT printf
#endif

#define NYAS__LOG_LOC NYAS_PRINT("%s(%u)", __FILE__, __LINE__)

#define NYAS_LOG(...)                                                                              \
    NYAS_PRINT("Nyas [Log] ");                                                                     \
    NYAS__LOG_LOC;                                                                                 \
    NYAS_PRINT(":\n\t");                                                                           \
    NYAS_PRINT(__VA_ARGS__);                                                                       \
    NYAS_PRINT("\n")

#define NYAS_LOG_WARN(...)                                                                         \
    NYAS_PRINT("Nyas [Warning] ");                                                                 \
    NYAS__LOG_LOC;                                                                                 \
    NYAS_PRINT(":\n\t");                                                                           \
    NYAS_PRINT(__VA_ARGS__);                                                                       \
    NYAS_PRINT("\n")

#define NYAS_LOG_ERR(...)                                                                          \
    NYAS_PRINT("Nyas [Error] ");                                                                   \
    NYAS__LOG_LOC;                                                                                 \
    NYAS_PRINT(":\n\t");                                                                           \
    NYAS_PRINT(__VA_ARGS__);                                                                       \
    NYAS_PRINT("\n")

#ifndef NyDrawIdx
typedef unsigned short NyDrawIdx;
#endif

typedef int NyasHandle;

// Basic data types
struct NyasCtx;
struct NyasPlatform;
struct NyasIO;
struct NyasResource;
struct NyasTexDesc;
struct NyasTexTarget;
struct NyasTexImg;
struct NyasShaderDesc;
struct NyasTexture;
struct NyasMesh;
struct NyasShader;
struct NyasMaterial;
struct NyasFramebuffer;
struct NyasDrawState;
struct NyasDrawUnit;
struct NyasDrawCmd;
struct NyasCamera;
struct NyasEntity;

// Flags
typedef int NyasResourceFlags; // enum NyasResourceFlags_
typedef int NyasTexFlags; // enum NyasTexFlags_
typedef int NyasVtxAttribFlags; // enum NyasVtxAttribFlags_
typedef int NyasDrawFlags; // enum NyasDrawFlags_

// Enum
typedef int NyasTexType; // enum NyasTexType_
typedef int NyasTexFmt; // enum NyasTexFmt_
typedef int NyasTexFilter; // enum NyasTexFilter_
typedef int NyasTexWrap; // enum NyasTexWrap_
typedef int NyasTexFace; // enum NyasTexFace_
typedef int NyasVtxAttrib; // enum NyasVtxAttrib_
typedef int NyasFbAttach; // enum NyasFbAttach_
typedef int NyasBlendFunc; // enum NyasBlendFunc_
typedef int NyasDepthFunc; // enum NyasDepthFunc_
typedef int NyasFaceCull; // enum NyasFaceCull_
typedef int NyasKey; // enum NyasKey_
typedef int NyasKeyState; // enum NyasKeyState_
typedef int NyasMouseButton; // enum NyasMouseButton_
typedef int NyasCode; // enum NyasCode_
typedef int NyasError; // enum NyasError_

namespace Nyas
{
NyasHandle CreateTexture(void);
void SetTexture(NyasHandle tex, struct NyasTexDesc *desc);
void LoadTexture(NyasHandle tex, struct NyasTexDesc *desc, const char *path);

NyasHandle CreateFramebuffer(void);
void SetFramebufferTarget(NyasHandle fb, int index, struct NyasTexTarget target);

// TODO(Renderer): Unificar load y reload
NyasHandle CreateMesh(void);
NyasHandle LoadMesh(const char *path);
void ReloadMesh(NyasHandle mesh, const char *path);

NyasHandle CreateShader(const struct NyasShaderDesc *desc);
void *GetMaterialSharedData(NyasHandle shader);
void ReloadShader(NyasHandle shader);
NyasHandle *GetMaterialSharedTextures(NyasHandle shader);
NyasHandle *GetMaterialSharedCubemaps(NyasHandle shader);

/* Creates a new material and alloc persistent memory for its data */
NyasMaterial CreateMaterial(NyasHandle shader);
/* Creates a new material and alloc frame-scoped memory for its data */
NyasMaterial CreateMaterialTmp(NyasHandle shader);
NyasMaterial CopyMaterialTmp(NyasMaterial mat);
NyasMaterial CopyShaderMaterialTmp(NyasHandle shader);
NyasHandle *GetMaterialTextures(NyasMaterial mat); // Ptr to first texture.

void Draw(NyasDrawCmd *command);

NyasCtx *GetCurrentCtx();

bool InitIO(const char *title, int win_w, int win_h);
void PollIO(void);
void WindowSwap(void);
int ReadFile(const char *path, char **dst, size_t *size);
} // namespace Nyas

struct NyAllocator
{
    static inline void *Alloc(size_t size, void *_ = NULL)
    {
        NY_UNUSED(_);
        return NYAS_ALLOC(size);
    }
    static inline void Free(void *ptr, void *_ = NULL)
    {
        NY_UNUSED(_);
        NYAS_FREE(ptr);
    }
};

template<size_t CAP> struct NyCircularAllocator
{
    static char Arena[CAP];
    static ptrdiff_t Offset;
    static inline void *Alloc(size_t size, void *_ = NULL)
    {
        NY_UNUSED(_);

        if (Offset + size > CAP)
        {
            Offset = size;
            return (void *)Arena;
        }

        void *ret = (void *)&Arena[Offset];
        Offset += size;
        return ret;
    }
    static inline void Free(void *ptr, void *_ = NULL)
    {
        NY_UNUSED(_);
        NY_UNUSED(ptr);
    }
};

template<size_t CAP> char NyCircularAllocator<CAP>::Arena[CAP] = { 0 };
template<size_t CAP> ptrdiff_t NyCircularAllocator<CAP>::Offset = { 0 };
typedef NyCircularAllocator<NYAS_FRAME_ALLOCATOR_ARENA_SIZE> NyFrameAllocator;

template<typename T, typename A = NyAllocator> struct NyBuffer
{
    T *Data;
    int Capacity;

    inline NyBuffer() : Data(NULL), Capacity(0) {}
    inline NyBuffer(int capacity) { Reserve(capacity); }
    inline ~NyBuffer()
    {
        A::Free(Data);
        Data = NULL;
        Capacity = 0;
    }
    inline void Reserve(int capacity)
    {
        if (capacity < Capacity)
        {
            return;
        }
        T *data = (T *)A::Alloc(capacity * sizeof(T));
        NYAS_ASSERT(data);
        memcpy(data, Data, sizeof(T) * Capacity);
        A::Free(Data);
        Data = data;
        Capacity = capacity;
    }

    inline const T &operator[](int i) const { return Data[i]; }
    inline T &operator[](int i) { return Data[i]; }
};

// Dynamic array.
template<typename T, typename A = NyAllocator> struct NyArray
{
    NyBuffer<T, A> Buf;
    int Size;

    inline NyArray() : Buf(), Size(0) {}
    inline NyArray(int capacity) : Buf(capacity), Size(0) {}
    inline ~NyArray() { Size = 0; }
    inline void Push(const T &value)
    {
        if (Buf.Capacity == Size)
        {
            Buf.Reserve(Size > 4 ? Size * 2 : 8);
        }
        Buf[Size] = value;
        ++Size;
    }
    inline void Pop()
    {
        NYAS_ASSERT(Size > 0);
        --Size;
    }
    inline const T &Back() { return Buf[Size - 1]; }
    inline const T &operator[](int i) const { return Buf[i]; }
    inline T &operator[](int i) { return Buf[i]; }
};

// Basic pool, uses internal array index as id (key).
template<typename T, typename A = NyAllocator> struct NyPool
{
    NyArray<T, A> Arr;
    int Count;
    int Next;

    inline NyPool() : Arr(), Count(0), Next(0) {}
    inline NyPool(int capacity) : Arr(capacity), Count(0), Next(0) {}
    inline ~NyPool()
    {
        Count = 0;
        Next = 0;
    }

    inline const T &operator[](int i) const { return Arr[i]; }
    inline T &operator[](int i) { return Arr[i]; }

    inline int Add(const T &value = T())
    {
        int ret = Next;
        if (Next == Arr.Size)
        {
            Arr.Push(value);
            ++Next;
        }
        else
        {
            Next = *(int *)&Arr[Next];
            Arr[ret] = value;
        }
        ++Count;
        return ret;
    }

    inline void Remove(int id)
    {
        *(int *)&Arr[id] = Next;
        Next = id;
        --Count;
    }
};

struct NyVec2i
{
    int X, Y;
    NyVec2i() : X(0), Y(0) {}
    NyVec2i(int x, int y) : X(x), Y(y) {}
    inline operator int *() { return &X; }
};

struct NyVec2
{
    float X, Y;
    NyVec2() : X(0.0f), Y(0.0f) {}
    NyVec2(float x, float y) : X(x), Y(y) {}
    NyVec2(const NyVec2 &other) { *this = other; }
    inline void operator=(const NyVec2 &other)
    {
        X = other.X;
        Y = other.Y;
    }
    inline operator float *() { return &X; }
};

struct NyVec3
{
    float X, Y, Z;
    static inline NyVec3 Up() { return { 0.0f, 1.0f, 0.0f }; }
    NyVec3() : X(0.0f), Y(0.0f), Z(0.0f) {}
    NyVec3(float x, float y, float z) : X(x), Y(y), Z(z) {}
    inline operator float *() { return &X; }
};

struct NyRect
{
    int X, Y, W, H;
    NyRect() : X(0), Y(0), W(0), H(0) {}
    NyRect(int x, int y, int w, int h) : X(x), Y(y), W(w), H(h) {}
    inline operator int *() { return &X; }
};

enum NyasTexType_
{
    NyasTexType_Default,
    NyasTexType_2D,
    NyasTexType_Array2D,
    NyasTexType_Cubemap,
    NyasTexType_ArrayCubemap,
    NyasTexType_COUNT
};

enum NyasTexFmt_
{
    NyasTexFmt_Default,
    NyasTexFmt_Depth,
    NyasTexFmt_Stencil,
    NyasTexFmt_DepthStencil,
    NyasTexFmt_R_8,
    NyasTexFmt_RG_8,
    NyasTexFmt_RGB_8,
    NyasTexFmt_RGBA_8,
    NyasTexFmt_SRGB_8,
    NyasTexFmt_R_16F,
    NyasTexFmt_RG_16F,
    NyasTexFmt_RGB_16F,
    NyasTexFmt_RGBA_16F,
    NyasTexFmt_R_32F,
    NyasTexFmt_RG_32F,
    NyasTexFmt_RGB_32F,
    NyasTexFmt_RGBA_32F,
    NyasTexFmt_COUNT
};

enum NyasTexFace_
{
    NyasTexFace_PosX,
    NyasTexFace_NegX,
    NyasTexFace_PosY,
    NyasTexFace_NegY,
    NyasTexFace_PosZ,
    NyasTexFace_NegZ,
    NyasTexFace_2D,
    NyasTexFace_Default,
    NyasTexFace_COUNT
};

enum NyasTexFilter_
{
    NyasTexFilter_Default,
    NyasTexFilter_Linear,
    NyasTexFilter_LinearMipmapLinear,
    NyasTexFilter_LinearMipmapNearest,
    NyasTexFilter_Nearest,
    NyasTexFilter_NearestMipmapLinear,
    NyasTexFilter_NearestMipmapNearest,
    NyasTexFilter_COUNT
};

enum NyasTexWrap_
{
    NyasTexWrap_Default,
    NyasTexWrap_Clamp,
    NyasTexWrap_Repeat,
    NyasTexWrap_Mirror,
    NyasTexWrap_White,
    NyasTexWrap_Black,
    NyasTexWrap_COUNT
};

enum NyasFbAttach_
{
    NyasFbAttach_DepthStencil = -3,
    NyasFbAttach_Stencil = -2,
    NyasFbAttach_Depth = -1,
    NyasFbAttach_Color = 0,
    NyasFbAttach_Color1,
    NyasFbAttach_Color2,
    NyasFbAttach_Color3,
    NyasFbAttach_Color4,
    NyasFbAttach_Color5,
    NyasFbAttach_Color6
};

enum NyasVtxAttrib_
{
    NyasVtxAttrib_Pos,
    NyasVtxAttrib_Normal,
    NyasVtxAttrib_Tan,
    NyasVtxAttrib_Bitan,
    NyasVtxAttrib_UV,
    NyasVtxAttrib_Color,
    NyasVtxAttrib_COUNT
};

enum NyasBlendFunc_
{
    NyasBlendFunc_Default,
    NyasBlendFunc_One,
    NyasBlendFunc_SrcAlpha,
    NyasBlendFunc_OneMinusSrcAlpha,
    NyasBlendFunc_Zero,
    NyasBlendFunc_COUNT
};

enum NyasDepthFunc_
{
    NyasDepthFunc_Default,
    NyasDepthFunc_Less,
    NyasDepthFunc_LessEqual,
    NyasDepthFunc_COUNT
};

enum NyasFaceCull_
{
    NyasFaceCull_Default,
    NyasFaceCull_Front,
    NyasFaceCull_Back,
    NyasFaceCull_FrontAndBack,
    NyasFaceCull_COUNT
};

enum NyasTexFlags_
{
    NyasTexFlags_None = 0,
    NyasTexFlags_GenMipMaps = 1,
    NyasTexFlags_FlipVerticallyOnLoad = 1 << 1
};

enum NyasDrawFlags_
{
    NyasDrawFlags_None = 0,
    NyasDrawFlags_ColorClear = 1,
    NyasDrawFlags_DepthClear = 1 << 1,
    NyasDrawFlags_StencilClear = 1 << 2,
    NyasDrawFlags_DepthTest = 1 << 3,
    NyasDrawFlags_StencilTest = 1 << 4,
    NyasDrawFlags_DepthWrite = 1 << 5,
    NyasDrawFlags_StencilWrite = 1 << 6,
    NyasDrawFlags_Blend = 1 << 7,
    NyasDrawFlags_FaceCulling = 1 << 8,
    NyasDrawFlags_Scissor = 1 << 9,
};

enum NyasVtxAttribFlags_
{
    NyasVtxAttribFlags_None = 0,
    NyasVtxAttribFlags_Position = 1,
    NyasVtxAttribFlags_Normal = 1 << 1,
    NyasVtxAttribFlags_Tangent = 1 << 2,
    NyasVtxAttribFlags_Bitangent = 1 << 3,
    NyasVtxAttribFlags_UV = 1 << 4,
    NyasVtxAttribFlags_Color = 1 << 5,
};

enum NyasResourceFlags_
{
    NyasResourceFlags_None = 0,
    NyasResourceFlags_Dirty = 1 << 3,
    NyasResourceFlags_Created = 1 << 4,
    NyasResourceFlags_ReleaseAppStorage = 1 << 5,
    NyasResourceFlags_Mapped = 1 << 7,
};

enum NyasKey_
{
    NyasKey_Invalid = 0,
    NyasKey_Space = 32,
    NyasKey_Apostrophe = 39, /* ' */
    NyasKey_Comma = 44, /* , */
    NyasKey_Minus = 45, /* - */
    NyasKey_Period = 46, /* . */
    NyasKey_Slash = 47, /* / */
    NyasKey_0 = 48,
    NyasKey_1 = 49,
    NyasKey_2 = 50,
    NyasKey_3 = 51,
    NyasKey_4 = 52,
    NyasKey_5 = 53,
    NyasKey_6 = 54,
    NyasKey_7 = 55,
    NyasKey_8 = 56,
    NyasKey_9 = 57,
    NyasKey_Semicolon = 59, /* ; */
    NyasKey_Equal = 61, /* = */
    NyasKey_A = 65,
    NyasKey_B = 66,
    NyasKey_C = 67,
    NyasKey_D = 68,
    NyasKey_E = 69,
    NyasKey_F = 70,
    NyasKey_G = 71,
    NyasKey_H = 72,
    NyasKey_I = 73,
    NyasKey_J = 74,
    NyasKey_K = 75,
    NyasKey_L = 76,
    NyasKey_M = 77,
    NyasKey_N = 78,
    NyasKey_O = 79,
    NyasKey_P = 80,
    NyasKey_Q = 81,
    NyasKey_R = 82,
    NyasKey_S = 83,
    NyasKey_T = 84,
    NyasKey_U = 85,
    NyasKey_V = 86,
    NyasKey_W = 87,
    NyasKey_X = 88,
    NyasKey_Y = 89,
    NyasKey_Z = 90,
    NyasKey_LeftBracket = 91, /* [ */
    NyasKey_Backslash = 92, /* \ */
    NyasKey_RightBracket = 93, /* ] */
    NyasKey_GraveAccent = 96, /* ` */
    NyasKey_World1 = 161, /* non-US #1 */
    NyasKey_World2 = 162, /* non-US #2 */
    NyasKey_Escape = 256,
    NyasKey_Enter = 257,
    NyasKey_Tab = 258,
    NyasKey_Backspace = 259,
    NyasKey_Insert = 260,
    NyasKey_Delete = 261,
    NyasKey_Right = 262,
    NyasKey_Left = 263,
    NyasKey_Down = 264,
    NyasKey_Up = 265,
    NyasKey_PageUp = 266,
    NyasKey_PageDown = 267,
    NyasKey_Home = 268,
    NyasKey_End = 269,
    NyasKey_CapsLock = 280,
    NyasKey_ScrollLock = 281,
    NyasKey_NumLock = 282,
    NyasKey_PrintScreen = 283,
    NyasKey_Pause = 284,
    NyasKey_F1 = 290,
    NyasKey_F2 = 291,
    NyasKey_F3 = 292,
    NyasKey_F4 = 293,
    NyasKey_F5 = 294,
    NyasKey_F6 = 295,
    NyasKey_F7 = 296,
    NyasKey_F8 = 297,
    NyasKey_F9 = 298,
    NyasKey_F10 = 299,
    NyasKey_F11 = 300,
    NyasKey_F12 = 301,
    NyasKey_F13 = 302,
    NyasKey_F14 = 303,
    NyasKey_F15 = 304,
    NyasKey_F16 = 305,
    NyasKey_F17 = 306,
    NyasKey_F18 = 307,
    NyasKey_F19 = 308,
    NyasKey_F20 = 309,
    NyasKey_F21 = 310,
    NyasKey_F22 = 311,
    NyasKey_F23 = 312,
    NyasKey_F24 = 313,
    NyasKey_F25 = 314,
    NyasKey_Kp0 = 320,
    NyasKey_Kp1 = 321,
    NyasKey_Kp2 = 322,
    NyasKey_Kp3 = 323,
    NyasKey_Kp4 = 324,
    NyasKey_Kp5 = 325,
    NyasKey_Kp6 = 326,
    NyasKey_Kp7 = 327,
    NyasKey_Kp8 = 328,
    NyasKey_Kp9 = 329,
    NyasKey_KpDecimal = 330,
    NyasKey_KpDivide = 331,
    NyasKey_KpMultiply = 332,
    NyasKey_KpSubtract = 333,
    NyasKey_KpAdd = 334,
    NyasKey_KpEnter = 335,
    NyasKey_KpEqual = 336,
    NyasKey_LeftShift = 340,
    NyasKey_LeftControl = 341,
    NyasKey_LeftAlt = 342,
    NyasKey_LeftSuper = 343,
    NyasKey_RightShift = 344,
    NyasKey_RightControl = 345,
    NyasKey_RightAlt = 346,
    NyasKey_RightSuper = 347,
    NyasKey_Menu = 348
};

enum NyasKeyState_
{
    NyasKeyState_RELEASED = 0, /* Not pressed */
    NyasKeyState_DOWN = 1, /* From released to pressed this frame */
    NyasKeyState_UP = 2, /* From pressed to released this frame */
    NyasKeyState_PRESSED = 3, /* Pressed */
};

enum NyasMouseButton_
{
    NyasMouseButton_Left,
    NyasMouseButton_Right,
    NyasMouseButton_Middle,
    NyasMouseButton_COUNT
};

enum NyasCode_
{
    NyasCode_Null = 0,
    NyasCode_Ok = 0,
    NyasCode_Error = -1,
    NyasCode_Default = -50,
    NyasCode_NoOp = -51,
    NyasCode_None = -53,
};

enum NyasError_
{
    NyasError_Memory = -100,
    NyasError_Null = -101,
    NyasError_Alloc = -110,
    NyasError_Range = -120, // Out of bounds access.
    NyasError_Stream = -200,
    NyasError_File = -210,
    NyasError_Thread = -300,
    NyasError_Arg = -400,
    NyasError_NullArg = -401,
    NyasError_BadArg = -402,
    NyasError_Switch = -500,
    NyasError_SwitchBadLabel = -501, // Usually unwanted default cases.
};

typedef struct NyasPlatform
{
    void *(*Alloc)(size_t size);
    void (*Free)(void *ptr);
    int64_t (*GetTime)(); // Get system time in nanoseconds.

    float DeltaTime;
    bool WindowClosed;
    bool WindowHovered;
    bool WindowFocused;
} NyasPlatform;

typedef struct NyasConfig
{
    struct
	{
		float Speed;
		float DragSensibility;
		float ScrollSensibility;
	} Navigation;
} NyasConfig;

typedef struct NyasIO
{
    NyasKeyState Keys[348 + 1]; // TODO: Get last key value from the enum.
    NyasKeyState MouseButton[3];
    void *InternalWindow;
    NyVec2i WindowSize; // In pixels
    NyVec2 MousePosition;
    NyVec2 MouseScroll; // x horizontal and y vertical scrolls
    bool ShowCursor;
    bool CaptureMouse;
    bool CaptureKeyboard;

    NyasIO() : InternalWindow(NULL) {}
} NyasIO;

typedef struct NyasCtx
{
    NyasConfig Cfg;
    NyasPlatform Platform;
    NyasIO IO;
} NyasCtx;

typedef struct NyasTexDesc
{
    int Width;
    int Height;
    NyasTexFlags Flags;
    NyasTexType Type;
    NyasTexFmt Format;
    NyasTexFilter MinFilter;
    NyasTexFilter MagFilter;
    NyasTexWrap WrapS;
    NyasTexWrap WrapT;
    NyasTexWrap WrapR;
    float BorderColor[4];

    NyasTexDesc() :
        Width(0), Height(0), Flags(NyasTexFlags_None), Type(NyasTexType_2D),
        Format(NyasTexFmt_SRGB_8), MinFilter(NyasTexFilter_Linear), MagFilter(NyasTexFilter_Linear),
        WrapS(NyasTexWrap_Repeat), WrapT(NyasTexWrap_Repeat), WrapR(NyasTexWrap_Repeat)
    {
        BorderColor[0] = 1.0f;
        BorderColor[1] = 1.0f;
        BorderColor[2] = 1.0f;
        BorderColor[3] = 1.0f;
    }

    NyasTexDesc(NyasTexType type, NyasTexFmt fmt, int w, int h) :
        Width(w), Height(h), Flags(NyasTexFlags_None), Type(type), Format(fmt),
        MinFilter(NyasTexFilter_Linear), MagFilter(NyasTexFilter_Linear), WrapS(NyasTexWrap_Repeat),
        WrapT(NyasTexWrap_Repeat), WrapR(NyasTexWrap_Repeat)
    {
        BorderColor[0] = 1.0f;
        BorderColor[1] = 1.0f;
        BorderColor[2] = 1.0f;
        BorderColor[3] = 1.0f;
    }
} NyasTexDesc;

typedef struct NyasTexTarget
{
    NyasHandle Tex;
    NyasTexFace Face;
    NyasFbAttach Attach;
    int MipLevel;
} NyasTexTarget;

typedef struct NyasTexImg
{
    void *Pix;
    NyasTexFace Face;
    int MipLevel;
    NyasTexImg() : Pix(NULL), Face(NyasTexFace_2D), MipLevel(0) {}
} NyasTexImg;

typedef struct NyasMaterial
{
    void *Ptr; // TODO deprecar
    NyasHandle Shader;
    NyasMaterial() : Ptr(NULL), Shader(NyasCode_NoOp) {}
    NyasMaterial(NyasHandle shader) : Ptr(NULL), Shader(shader) {}
} NyasMaterial;

typedef struct NyasResource
{
    uint32_t Id;
    NyasResourceFlags Flags;

    NyasResource() : Id(0), Flags(0) {}
} NyasResource;

typedef struct NyasShaderDesc
{
    const char *Name;
    int DataCount;
    int TexCount;
    int CubemapCount;
    int SharedDataCount;
    int SharedTexCount;
    int SharedCubemapCount;

    NyasShaderDesc(const char *id, int dcount, int tcount, int cmcount, int sdcount, int stcount,
        int scmcount) :
        Name(id),
        DataCount(dcount), TexCount(tcount), CubemapCount(cmcount), SharedDataCount(sdcount),
        SharedTexCount(stcount), SharedCubemapCount(scmcount)
    {
    }
} NyasShaderDesc;

typedef struct NyasTexture
{
    NyasResource Resource;
    NyasTexDesc Data;
    NyArray<NyasTexImg> Img;
} NyasTexture;

typedef struct NyasMesh
{
    NyasResource Resource;
    NyasResource ResVtx; // vertex buffer resource
    NyasResource ResIdx; // index buffer resource
    float *Vtx;
    NyDrawIdx *Indices;
    int64_t ElementCount;
    uint32_t VtxSize;
    NyasVtxAttribFlags Attribs;
} NyasMesh;

typedef struct NyasShader
{
    NyasResource Resource;
    const char *Name;
    struct
    {
        int Data, Tex, Cubemap;
    } Location[2], Count[2]; // 0: unit, 1: common
    void *Shared;
    void **Unit;
} NyasShader;

typedef struct NyasFramebuffer
{
    NyasResource Resource;
    NyasTexTarget Target[8];
} NyasFramebuffer;

typedef struct NyasDrawState
{
    NyasBlendFunc BlendSrc;
    NyasBlendFunc BlendDst;
    NyasDepthFunc Depth;
    NyasFaceCull FaceCulling;
    NyasDrawFlags EnableFlags;
    NyasDrawFlags DisableFlags;
    int ViewportMinX; // if (viewport_min_x == viewport_max_x): no-op
    int ViewportMinY;
    int ViewportMaxX;
    int ViewportMaxY;
    int ScissorMinX; // if (scissor_min_x == scissor_max_x): no-op
    int ScissorMinY;
    int ScissorMaxX;
    int ScissorMaxY;
    float BgColorR; // if (bg_color_a == -1.0f): no-op
    float BgColorG;
    float BgColorB;
    float BgColorA;

    NyasDrawState() :
        BlendSrc(NyasBlendFunc_Default), BlendDst(NyasBlendFunc_Default),
        Depth(NyasDepthFunc_Default), FaceCulling(NyasFaceCull_Default),
        EnableFlags(NyasDrawFlags_None), DisableFlags(NyasDrawFlags_None), ViewportMinX(0),
        ViewportMinY(0), ViewportMaxX(0), ViewportMaxY(0), ScissorMinX(0), ScissorMinY(0),
        ScissorMaxX(0), ScissorMaxY(0), BgColorR(0.0f), BgColorG(0.0f), BgColorB(0.0f),
        BgColorA(-1.0f)
    {
    }
} NyasDrawState;

typedef struct NyasDrawUnit
{
    NyasMaterial Material;
    NyasHandle Mesh;
} NyasDrawUnit;

typedef struct NyasDrawCmd
{
    NyasDrawState State;
    NyasDrawUnit *Units;
    int UnitCount;
    NyasHandle Framebuf;
    NyasMaterial ShaderMaterial;
    NyasDrawCmd() : Units(NULL), UnitCount(0), Framebuf(NyasCode_NoOp) {}
} NyasDrawCmd;

typedef struct NyasCamera
{
    float View[16];
    float Proj[16];
    float Far;
    float Fov;
    NyasCamera() { memset(this, 0, sizeof(*this)); }
    void Navigate();

    // Position.
    inline NyVec3 Eye() const
    {
        float inv[16];
        mat4_inverse(inv, View);
        return { inv[12], inv[13], inv[14] };
    }

    inline NyVec3 Fwd() const { return { View[2], View[6], View[10] }; } // Forward vector.

    // Matrix with zeroed translation (i.e., projection * vec4(vec3(view)). For skybox.
    inline float *OriginViewProj(float out[16])
    {
        mat4_assign(out, View);
        out[3] = 0.0f;
        out[7] = 0.0f;
        out[11] = 0.0f;
        out[12] = 0.0f;
        out[13] = 0.0f;
        out[14] = 0.0f;
        out[15] = 0.0f;
        return mat4_multiply(out, Proj, out);
    }

    inline void Init(const NyasIO &io, NyVec3 pos = { 0.0f, 2.0f, 2.0f },
        NyVec3 target = { 0.0f, 0.0f, -1.0f }, float far = 300.0f, float fov = 70.0f)
    {
        Far = far;
        Fov = fov;
        mat4_look_at(View, pos, target, NyVec3::Up());
        mat4_perspective_fov(Proj, to_radians(Fov), io.WindowSize.X, io.WindowSize.Y, 0.01f, Far);
    }
} NyasCamera;

typedef struct NyasEntity
{
    float Transform[16];
    NyasHandle Mesh;
    NyasMaterial Material;
} NyasEntity;

namespace Nyas
{
extern NyPool<NyasMesh> Meshes;
extern NyPool<NyasTexture> Textures;
extern NyPool<NyasShader> Shaders;
extern NyPool<NyasFramebuffer> Framebufs;
extern NyPool<NyasEntity> Entities;
extern NyasCamera Camera;
} // namespace Nyas

// ---
// [UTILS]
// ---

// Nanosecond timer
struct NyChrono
{
    int64_t Start { 0 };

    static inline int64_t GetTime() { return NYAS_GET_TIME_NS; } // Current time in nanoseconds.
    static constexpr int64_t MicroSeconds(int64_t ns) { return ns / 1000; }
    static constexpr int64_t MilliSeconds(int64_t ns) { return MicroSeconds(ns) / 1000; }
    static constexpr int64_t Seconds(int64_t ns) { return MilliSeconds(ns) / 1000; }
    static constexpr double MicroSeconds(double ns) { return ns / 1000.0; }
    static constexpr double MilliSeconds(double ns) { return MicroSeconds(ns) / 1000.0; }
    static constexpr double Seconds(double ns) { return MilliSeconds(ns) / 1000.0; }

    NyChrono() : Start(GetTime()) {}
    NyChrono(int64_t start_ns) : Start(start_ns) {}
    void Restart() { Start = GetTime(); }
    int64_t Elapsed() { return GetTime() - Start; }
};

struct NySched
{
    struct Job
    {
        void (*Func)(void *);
        void *Args;
        Job() : Func(NULL), Args(NULL) {}
        Job(void (*func)(void *), void *args) : Func(func), Args(args) {}
    };

    struct _NyScheduler *_Sched;

    NySched(int thread_count);
    ~NySched();
    void Do(Job job);
    void Wait();
};

struct NyAssetLoader
{
    struct TexArgs
    {
        NyasTexDesc Descriptor;
        const char *Path;
        NyasHandle Tex;
    };

    struct MeshArgs
    {
        const char *Path;
        NyasHandle *Mesh;
    };

    struct ShaderArgs
    {
        NyasShaderDesc Descriptor;
        NyasHandle *Shader;
    };

    struct EnvArgs
    {
        const char *Path;
        NyasHandle *Sky;
        NyasHandle *Irradiance;
        NyasHandle *Pref;
        NyasHandle *LUT;
    };

    NyArray<NySched::Job> Sequential;
    NyArray<NySched::Job> Async;
    void AddMesh(MeshArgs *args);
    void AddTex(TexArgs *args);
    void AddShader(ShaderArgs *args);
    void AddEnv(EnvArgs *args);
    void AddJob(NySched::Job job, bool async);
    void Load(int threads);
};

enum NyasGeometry
{
    NyasGeometry_Quad,
    NyasGeometry_Cube,
    NyasGeometry_Sphere,
    NyasGeometry_COUNT
};

namespace NyUtil
{
void LoadBasicGeometries(void);

// Environment maps
void LoadEnv(const char *path, NyasHandle *lut, NyasHandle *sky, NyasHandle *irr, NyasHandle *pref);

} // namespace NyUtil

// Geometry
extern NyasHandle NYAS_SPHERE;
extern NyasHandle NYAS_CUBE;
extern NyasHandle NYAS_QUAD;

#endif // NYAS_H
