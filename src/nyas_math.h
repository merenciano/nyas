#ifndef NYAS_MATH_H
#define NYAS_MATH_H

#include <mathc.h>

struct NyVec2i
{
	int X, Y;

	NyVec2i() : X(0), Y(0)
	{}

	NyVec2i(int x, int y) : X(x), Y(y)
	{}

	inline operator int *()
	{ return &X; }
};

struct NyVec2
{
	float X, Y;

	NyVec2() : X(0.0f), Y(0.0f)
	{}

	NyVec2(float x, float y) : X(x), Y(y)
	{}

	NyVec2(const NyVec2& other)
	{ *this = other; }

	inline void operator=(const NyVec2& other)
	{
		X = other.X;
		Y = other.Y;
	}

	inline operator float *()
	{ return &X; }
};

struct NyVec3
{
	float X, Y, Z;

	static inline NyVec3 Up()
	{ return {0.0f, 1.0f, 0.0f}; }

	NyVec3() : X(0.0f), Y(0.0f), Z(0.0f)
	{}

	NyVec3(float x, float y, float z) : X(x), Y(y), Z(z)
	{}

	inline operator float *()
	{ return &X; }
};

struct NyRect
{
	int X, Y, W, H;

	NyRect() : X(0), Y(0), W(0), H(0)
	{}

	NyRect(int x, int y, int w, int h) : X(x), Y(y), W(w), H(h)
	{}

	inline operator int *()
	{ return &X; }
};

#include <array>

struct NyMat4
{
	static inline NyMat4 Identity()
	{ return {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}; }

	static inline NyMat4 LookAt(NyVec3 *pos, NyVec3 *target, NyVec3 *up)
	{ return {}; }

	static inline NyMat4 Ortho(float left, float right, float bottom, float top, float near, float far)
	{ return {}; }

	static inline NyMat4 Perspective(float fov_y, float aspect_ratio, float near, float far)
	{ return {}; }

	static inline NyMat4 PerspectiveFov(float fov, float width, float height, float near, float far)
	{ return {}; }

	float V00 = 0.0f, V10 = 0.0f, V20 = 0.0f, V30 = 0.0f;
	float V01 = 0.0f, V11 = 0.0f, V21 = 0.0f, V31 = 0.0f;
	float V02 = 0.0f, V12 = 0.0f, V22 = 0.0f, V32 = 0.0f;
	float V03 = 0.0f, V13 = 0.0f, V23 = 0.0f, V33 = 0.0f;

	NyMat4& operator=(const NyMat4& other) = default;

	void RotateX(float rad)
	{
		float c = cosf(rad);
		float s = sinf(rad);
		auto rot = Identity();
		rot[5] *= c;
		rot[6] *= s;
		rot[9] *= -s;
		rot[10] *= c;
		*this *= rot;
	}

	void RotateY(float rad)
	{
		float c = cosf(rad);
		float s = sinf(rad);
		auto rot = Identity();
		rot[0] = c;
		rot[2] = -s;
		rot[8] = s;
		rot[10] = c;
		*this *= rot;
	}

	void RotateZ(float rad)
	{
		float c = cosf(rad);
		float s = sinf(rad);
		auto rot = Identity();
		rot[0] = c;
		rot[1] = s;
		rot[4] = -s;
		rot[5] = c;
		*this *= rot;
	}

	void Translate(NyVec3 offset)
	{
		(&this->V00)[12] += offset.X;
		(&this->V00)[13] += offset.Y;
		(&this->V00)[14] += offset.Z;
	}

	void Scale(NyVec3 factor)
	{
		(&this->V00)[0] *= factor.X;
		(&this->V00)[5] *= factor.Y;
		(&this->V00)[10] *= factor.Z;
	}

	NyMat4 operator*(const NyMat4& other) const;
	const NyMat4& operator*=(const NyMat4& other) { return *this = *this * other; }
	NyMat4 operator*(float k) const;
	const NyMat4& operator*=(float k) { return *this = *this * k; }

	void Inverse()
	{}

	NyMat4 GetInverse()
	{ return {}; }

	explicit constexpr operator float *()
	{ return &V00; }

	constexpr float operator[](int i) const
	{ return (&V00)[i]; }

	float &operator[](int i)
	{ return (&V00)[i]; }
};

static inline NyMat4 operator*(const NyMat4 &lhs, const NyMat4 &rhs)
{
	return lhs.operator*(rhs);
}

static inline NyMat4 operator*(const NyMat4 &mat, float k)
{
	return mat.operator*(k);
}

inline NyMat4 NyMat4::operator*(const NyMat4& o) const
{
	return {
		this->operator[](0) * o[0] + this->operator[](4) * o[1] + this->operator[](8) * o[2] + this->operator[](12) * o[3],
		this->operator[](1) * o[0] + this->operator[](5) * o[1] + this->operator[](9) * o[2] + this->operator[](13) * o[3],
		this->operator[](2) * o[0] + this->operator[](6) * o[1] + this->operator[](10) * o[2] + this->operator[](14) * o[3],
		this->operator[](3) * o[0] + this->operator[](7) * o[1] + this->operator[](11) * o[2] + this->operator[](15) * o[3],
		this->operator[](0) * o[4] + this->operator[](4) * o[5] + this->operator[](8) * o[6] + this->operator[](12) * o[7],
		this->operator[](1) * o[4] + this->operator[](5) * o[5] + this->operator[](9) * o[6] + this->operator[](13) * o[7],
		this->operator[](2) * o[4] + this->operator[](6) * o[5] + this->operator[](10) * o[6] + this->operator[](14) * o[7],
		this->operator[](3) * o[4] + this->operator[](7) * o[5] + this->operator[](11) * o[6] + this->operator[](15) * o[7],
		this->operator[](0) * o[8] + this->operator[](4) * o[9] + this->operator[](8) * o[10] + this->operator[](12) * o[11],
		this->operator[](1) * o[8] + this->operator[](5) * o[9] + this->operator[](9) * o[10] + this->operator[](13) * o[11],
		this->operator[](2) * o[8] + this->operator[](6) * o[9] + this->operator[](10) * o[10] + this->operator[](14) * o[11],
		this->operator[](3) * o[8] + this->operator[](7) * o[9] + this->operator[](11) * o[10] + this->operator[](15) * o[11],
		this->operator[](0) * o[12] + this->operator[](4) * o[13] + this->operator[](8) * o[14] + this->operator[](12) * o[15],
		this->operator[](1) * o[12] + this->operator[](5) * o[13] + this->operator[](9) * o[14] + this->operator[](13) * o[15],
		this->operator[](2) * o[12] + this->operator[](6) * o[13] + this->operator[](10) * o[14] + this->operator[](14) * o[15],
		this->operator[](3) * o[12] + this->operator[](7) * o[13] + this->operator[](11) * o[14] + this->operator[](15) * o[15]
	};
}

inline NyMat4 NyMat4::operator*(float k) const
{
	return {
			this->operator[](0) * k,
			this->operator[](1) * k,
			this->operator[](2) * k,
			this->operator[](3) * k,
			this->operator[](4) * k,
			this->operator[](5) * k,
			this->operator[](6) * k,
			this->operator[](7) * k,
			this->operator[](8) * k,
			this->operator[](9) * k,
			this->operator[](10) * k,
			this->operator[](11) * k,
			this->operator[](12) * k,
			this->operator[](13) * k,
			this->operator[](14) * k,
			this->operator[](15) * k
	};
}

#endif
