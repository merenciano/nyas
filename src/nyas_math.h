#ifndef NYAS_MATH_H
#define NYAS_MATH_H

#include <mathc.h>

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

#endif // NYAS_MATH_H