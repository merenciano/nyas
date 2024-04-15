#ifndef NYAS_PLATFORM_H
#define NYAS_PLATFORM_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "nyconfig.h"
#include "nyas_math.h"

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

#define NY_UNUSED(_VAR) (void)(_VAR)
#define NY_KILOBYTES(_BYTES) ((_BYTES) * 1024L)
#define NY_MEGABYTES(_BYTES) (NY_KILOBYTES(_BYTES) * 1024L)
#define NY_GIGABYTES(_BYTES) (NYAS_MB(_BYTES) * 1024L)

#ifndef NYAS_GET_TIME_NS
#include <chrono>
#define NYAS_GET_TIME_NS (std::chrono::system_clock::now().time_since_epoch().count())
#endif

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


struct NyasPlatform;
struct NyasIO;

typedef int NyasKey; // enum NyasKey_
typedef int NyasKeyState; // enum NyasKeyState_
typedef int NyasMouseButton; // enum NyasMouseButton_
typedef int NyasCode; // enum NyasCode_
typedef int NyasError; // enum NyasError_

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
	int (*ReadFile)(const char *path, char **out_dst, size_t *out_size);

    float DeltaTime;
	void *InternalWindow;
    NyVec2i WindowSize; // In pixels
    bool WindowClosed;
    bool WindowHovered;
    bool WindowFocused;

	bool ShowCursor;
    bool CaptureMouse;
    bool CaptureKeyboard;

	NyasPlatform() : DeltaTime(1.0f / 60.0f), InternalWindow(NULL), WindowClosed(false), ShowCursor(true) {}
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
    NyVec2 MousePosition;
    NyVec2 MouseScroll; // x horizontal and y vertical scrolls
} NyasIO;

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

#endif // NYAS_PLATFORM_H
