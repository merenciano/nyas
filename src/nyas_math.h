#ifndef NYAS_MATH_H
#define NYAS_MATH_H

#include <math.h>
#include <string.h>

static inline float *vec3_subtract(float *result, const float *v0,
                                   const float *v1) {
  result[0] = v0[0] - v1[0];
  result[1] = v0[1] - v1[1];
  result[2] = v0[2] - v1[2];
  return result;
}

static inline float *vec3_cross(float *result, const float *v0,
                                const float *v1) {
  result[0] = v0[1] * v1[2] - v0[2] * v1[1];
  result[1] = v0[2] * v1[0] - v0[0] * v1[2];
  result[2] = v0[0] * v1[1] - v0[1] * v1[0];
  return result;
}

static inline float *vec3_normalize(float *result, const float *v0) {
  float l = sqrtf(v0[0] * v0[0] + v0[1] * v0[1] + v0[2] * v0[2]);
  result[0] = v0[0] / l;
  result[1] = v0[1] / l;
  result[2] = v0[2] / l;
  return result;
}

static inline float vec3_dot(const float *v0, const float *v1) {
  return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}

static inline float *mat4_assign(float *result, const float *m0) {
  result[0] = m0[0];
  result[1] = m0[1];
  result[2] = m0[2];
  result[3] = m0[3];
  result[4] = m0[4];
  result[5] = m0[5];
  result[6] = m0[6];
  result[7] = m0[7];
  result[8] = m0[8];
  result[9] = m0[9];
  result[10] = m0[10];
  result[11] = m0[11];
  result[12] = m0[12];
  result[13] = m0[13];
  result[14] = m0[14];
  result[15] = m0[15];
  return result;
}

static inline float *mat4_multiply(float *result, const float *m0,
                                   const float *m1) {
  result[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
  result[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
  result[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
  result[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
  result[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
  result[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
  result[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
  result[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
  result[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
  result[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
  result[10] =
      m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
  result[11] =
      m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
  result[12] =
      m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
  result[13] =
      m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
  result[14] =
      m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
  result[15] =
      m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
  return result;
}

static inline float *mat4_multiply_f(float *result, const float *m0, float f) {
  result[0] = m0[0] * f;
  result[1] = m0[1] * f;
  result[2] = m0[2] * f;
  result[3] = m0[3] * f;
  result[4] = m0[4] * f;
  result[5] = m0[5] * f;
  result[6] = m0[6] * f;
  result[7] = m0[7] * f;
  result[8] = m0[8] * f;
  result[9] = m0[9] * f;
  result[10] = m0[10] * f;
  result[11] = m0[11] * f;
  result[12] = m0[12] * f;
  result[13] = m0[13] * f;
  result[14] = m0[14] * f;
  result[15] = m0[15] * f;
  return result;
}

static inline float *mat4_inverse(float *result, const float *m0) {
  float inverse[16];
  float inverted_determinant;
  float m11 = m0[0];
  float m21 = m0[1];
  float m31 = m0[2];
  float m41 = m0[3];
  float m12 = m0[4];
  float m22 = m0[5];
  float m32 = m0[6];
  float m42 = m0[7];
  float m13 = m0[8];
  float m23 = m0[9];
  float m33 = m0[10];
  float m43 = m0[11];
  float m14 = m0[12];
  float m24 = m0[13];
  float m34 = m0[14];
  float m44 = m0[15];
  inverse[0] = m22 * m33 * m44 - m22 * m43 * m34 - m23 * m32 * m44 +
               m23 * m42 * m34 + m24 * m32 * m43 - m24 * m42 * m33;
  inverse[4] = -m12 * m33 * m44 + m12 * m43 * m34 + m13 * m32 * m44 -
               m13 * m42 * m34 - m14 * m32 * m43 + m14 * m42 * m33;
  inverse[8] = m12 * m23 * m44 - m12 * m43 * m24 - m13 * m22 * m44 +
               m13 * m42 * m24 + m14 * m22 * m43 - m14 * m42 * m23;
  inverse[12] = -m12 * m23 * m34 + m12 * m33 * m24 + m13 * m22 * m34 -
                m13 * m32 * m24 - m14 * m22 * m33 + m14 * m32 * m23;
  inverse[1] = -m21 * m33 * m44 + m21 * m43 * m34 + m23 * m31 * m44 -
               m23 * m41 * m34 - m24 * m31 * m43 + m24 * m41 * m33;
  inverse[5] = m11 * m33 * m44 - m11 * m43 * m34 - m13 * m31 * m44 +
               m13 * m41 * m34 + m14 * m31 * m43 - m14 * m41 * m33;
  inverse[9] = -m11 * m23 * m44 + m11 * m43 * m24 + m13 * m21 * m44 -
               m13 * m41 * m24 - m14 * m21 * m43 + m14 * m41 * m23;
  inverse[13] = m11 * m23 * m34 - m11 * m33 * m24 - m13 * m21 * m34 +
                m13 * m31 * m24 + m14 * m21 * m33 - m14 * m31 * m23;
  inverse[2] = m21 * m32 * m44 - m21 * m42 * m34 - m22 * m31 * m44 +
               m22 * m41 * m34 + m24 * m31 * m42 - m24 * m41 * m32;
  inverse[6] = -m11 * m32 * m44 + m11 * m42 * m34 + m12 * m31 * m44 -
               m12 * m41 * m34 - m14 * m31 * m42 + m14 * m41 * m32;
  inverse[10] = m11 * m22 * m44 - m11 * m42 * m24 - m12 * m21 * m44 +
                m12 * m41 * m24 + m14 * m21 * m42 - m14 * m41 * m22;
  inverse[14] = -m11 * m22 * m34 + m11 * m32 * m24 + m12 * m21 * m34 -
                m12 * m31 * m24 - m14 * m21 * m32 + m14 * m31 * m22;
  inverse[3] = -m21 * m32 * m43 + m21 * m42 * m33 + m22 * m31 * m43 -
               m22 * m41 * m33 - m23 * m31 * m42 + m23 * m41 * m32;
  inverse[7] = m11 * m32 * m43 - m11 * m42 * m33 - m12 * m31 * m43 +
               m12 * m41 * m33 + m13 * m31 * m42 - m13 * m41 * m32;
  inverse[11] = -m11 * m22 * m43 + m11 * m42 * m23 + m12 * m21 * m43 -
                m12 * m41 * m23 - m13 * m21 * m42 + m13 * m41 * m22;
  inverse[15] = m11 * m22 * m33 - m11 * m32 * m23 - m12 * m21 * m33 +
                m12 * m31 * m23 + m13 * m21 * m32 - m13 * m31 * m22;
  inverted_determinant = 1.0f / (m11 * inverse[0] + m21 * inverse[4] +
                                 m31 * inverse[8] + m41 * inverse[12]);
  result[0] = inverse[0] * inverted_determinant;
  result[1] = inverse[1] * inverted_determinant;
  result[2] = inverse[2] * inverted_determinant;
  result[3] = inverse[3] * inverted_determinant;
  result[4] = inverse[4] * inverted_determinant;
  result[5] = inverse[5] * inverted_determinant;
  result[6] = inverse[6] * inverted_determinant;
  result[7] = inverse[7] * inverted_determinant;
  result[8] = inverse[8] * inverted_determinant;
  result[9] = inverse[9] * inverted_determinant;
  result[10] = inverse[10] * inverted_determinant;
  result[11] = inverse[11] * inverted_determinant;
  result[12] = inverse[12] * inverted_determinant;
  result[13] = inverse[13] * inverted_determinant;
  result[14] = inverse[14] * inverted_determinant;
  result[15] = inverse[15] * inverted_determinant;
  return result;
}

static inline float *mat4_look_at(float *result, float *position, float *target,
                                  float *up) {
  float tmp_forward[3];
  float tmp_side[3];
  float tmp_up[3];
  vec3_subtract(tmp_forward, target, position);
  vec3_normalize(tmp_forward, tmp_forward);
  vec3_cross(tmp_side, tmp_forward, up);
  vec3_normalize(tmp_side, tmp_side);
  vec3_cross(tmp_up, tmp_side, tmp_forward);
  result[0] = tmp_side[0];
  result[1] = tmp_up[0];
  result[2] = -tmp_forward[0];
  result[3] = 0.0f;
  result[4] = tmp_side[1];
  result[5] = tmp_up[1];
  result[6] = -tmp_forward[1];
  result[7] = 0.0f;
  result[8] = tmp_side[2];
  result[9] = tmp_up[2];
  result[10] = -tmp_forward[2];
  result[11] = 0.0f;
  result[12] = -vec3_dot(tmp_side, position);
  result[13] = -vec3_dot(tmp_up, position);
  result[14] = vec3_dot(tmp_forward, position);
  result[15] = 1.0f;
  return result;
}

static inline float *mat4_perspective(float *result, float fov_y, float aspect,
                                      float n, float f) {
  float tan_half_fov_y = 1.0f / tanf(fov_y * 0.5f);
  result[0] = 1.0f / aspect * tan_half_fov_y;
  result[1] = 0.0f;
  result[2] = 0.0f;
  result[3] = 0.0f;
  result[4] = 0.0f;
  result[5] = 1.0f / tan_half_fov_y;
  result[6] = 0.0f;
  result[7] = 0.0f;
  result[8] = 0.0f;
  result[9] = 0.0f;
  result[10] = f / (n - f);
  result[11] = -1.0f;
  result[12] = 0.0f;
  result[13] = 0.0f;
  result[14] = -(f * n) / (f - n);
  result[15] = 0.0f;
  return result;
}

static inline float *mat4_perspective_fov(float *result, float fov, float w,
                                          float h, float n, float f) {
  float h2 = cosf(fov * 0.5f) / sinf(fov * 0.5f);
  float w2 = h2 * h / w;
  result[0] = w2;
  result[1] = 0.0f;
  result[2] = 0.0f;
  result[3] = 0.0f;
  result[4] = 0.0f;
  result[5] = h2;
  result[6] = 0.0f;
  result[7] = 0.0f;
  result[8] = 0.0f;
  result[9] = 0.0f;
  result[10] = f / (n - f);
  result[11] = -1.0f;
  result[12] = 0.0f;
  result[13] = 0.0f;
  result[14] = -(f * n) / (f - n);
  result[15] = 0.0f;
  return result;
}

namespace nym {

constexpr float PI = {3.1415926536f};

constexpr float to_radians(float degrees) { return degrees * PI / 180.0f; }

constexpr float clamp(float value, float min, float max) {
  return value < min ? min : value > max ? max : value;
}

struct vec2i_t {
	int x, y;
};

struct vec2_t {
  static constexpr float dot(vec2_t lhs, vec2_t rhs);

  static constexpr float magnitude(vec2_t vec);

  static constexpr vec2_t normal(vec2_t vec);

  constexpr vec2_t() : x(0.0f), y(0.0f) {}

  constexpr vec2_t(float ax, float ay) : x(ax), y(ay) {}

  constexpr vec2_t(float v[2]) : x(v[0]), y(v[1]) {}

  constexpr vec2_t(const vec2_t &other) : x(other.x), y(other.y) {}

  constexpr vec2_t &operator=(const vec2_t &other);

  constexpr operator float *();

  constexpr operator const float *() const;

  constexpr float operator[](int i) const;

  constexpr float &operator[](int i);

  constexpr vec2_t operator+(const vec2_t &other) const;

  constexpr void operator+=(const vec2_t &other);

  constexpr vec2_t operator-(const vec2_t &other) const;

  constexpr void operator-=(const vec2_t &other);

  constexpr vec2_t operator*(const vec2_t &other) const;

  constexpr void operator*=(const vec2_t &other);

  constexpr vec2_t operator*(float k) const;

  constexpr void operator*=(float k);

  constexpr vec2_t operator/(const vec2_t &other) const;

  constexpr void operator/=(const vec2_t &other);

  constexpr vec2_t operator/(float k) const;

  constexpr void operator/=(float k);

  void normalize();

  float x, y;
};

constexpr float vec2_t::dot(vec2_t lhs, vec2_t rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

constexpr float vec2_t::magnitude(vec2_t vec) { return sqrtf(dot(vec, vec)); }

constexpr vec2_t vec2_t::normal(vec2_t vec) { return vec / magnitude(vec); }

constexpr vec2_t::operator float *() { return &x; }

constexpr vec2_t::operator const float *() const { return &x; }

constexpr float vec2_t::operator[](int i) const { return (&x)[i]; }

constexpr float &vec2_t::operator[](int i) { return (&x)[i]; }

constexpr inline vec2_t &vec2_t::operator=(const vec2_t &other) {
  x = other.x, y = other.y;
  return *this;
}

constexpr inline void vec2_t::operator+=(const vec2_t &other) {
  *this = *this + other;
}

constexpr inline vec2_t vec2_t::operator+(const vec2_t &other) const {
  return {x + other.x, y + other.y};
}

constexpr inline void vec2_t::operator-=(const vec2_t &other) {
  *this = *this - other;
}

constexpr inline vec2_t vec2_t::operator-(const vec2_t &other) const {
  return {x - other.x, y - other.y};
}

constexpr inline void vec2_t::operator*=(const vec2_t &other) {
  *this = *this * other;
}

constexpr inline vec2_t vec2_t::operator*(const vec2_t &other) const {
  return {x * other.x, y * other.y};
}

constexpr inline void vec2_t::operator*=(float k) { *this = *this * k; }

constexpr inline vec2_t vec2_t::operator*(float k) const {
  return {x * k, y * k};
}

constexpr inline void vec2_t::operator/=(const vec2_t &other) {
  *this = *this / other;
}

constexpr inline vec2_t vec2_t::operator/(const vec2_t &other) const {
  return {x / other.x, y / other.y};
}

constexpr inline void vec2_t::operator/=(float k) { *this = *this / k; }

constexpr inline vec2_t vec2_t::operator/(float k) const {
  return *this * (1.0f / k);
}

inline void vec2_t::normalize() { *this = normal(*this); }

struct vec3_t {
  static constexpr vec3_t up();

  static constexpr float dot(vec3_t lhs, vec3_t rhs);

  static constexpr vec3_t cross(vec3_t lhs, vec3_t rhs);

  static constexpr float magnitude(vec3_t vec);

  static constexpr vec3_t normal(vec3_t vec);

  constexpr vec3_t(float ax, float ay, float az) : x(ax), y(ay), z(az) {}

  constexpr vec3_t(float v[3]) : x(v[0]), y(v[1]), z(v[2]) {}

  constexpr vec3_t(const vec3_t &other) : x(other.x), y(other.y), z(other.z) {}

  constexpr vec3_t &operator=(const vec3_t &other);

  constexpr operator float *();

  constexpr operator const float *() const;

  constexpr float operator[](int i) const;

  constexpr float &operator[](int i);

  constexpr vec3_t operator+(const vec3_t &other) const;

  constexpr void operator+=(const vec3_t &other);

  constexpr vec3_t operator-(const vec3_t &other) const;

  constexpr void operator-=(const vec3_t &other);

  constexpr vec3_t operator*(const vec3_t &other) const;

  constexpr void operator*=(const vec3_t &other);

  constexpr vec3_t operator*(float k) const;

  constexpr void operator*=(float k);

  constexpr vec3_t operator/(const vec3_t &other) const;

  constexpr void operator/=(const vec3_t &other);

  constexpr vec3_t operator/(float k) const;

  constexpr void operator/=(float k);

  void normalize();

  float x, y, z;
};

constexpr vec3_t vec3_t::up() { return {0.0f, 1.0f, 0.0f}; }

constexpr float vec3_t::dot(vec3_t lhs, vec3_t rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

constexpr vec3_t vec3_t::cross(vec3_t lhs, vec3_t rhs) {
  return {lhs[1] * rhs[2] - lhs[2] * rhs[1], lhs[2] * rhs[0] - lhs[0] * rhs[2],
          lhs[0] * rhs[1] - lhs[1] * rhs[0]};
}

constexpr float vec3_t::magnitude(vec3_t vec) { return sqrtf(dot(vec, vec)); }

constexpr vec3_t vec3_t::normal(vec3_t vec) { return vec / magnitude(vec); }

constexpr vec3_t::operator float *() { return &x; }

constexpr vec3_t::operator const float *() const { return &x; }

constexpr float vec3_t::operator[](int i) const { return (&x)[i]; }

constexpr float &vec3_t::operator[](int i) { return (&x)[i]; }

constexpr inline vec3_t &vec3_t::operator=(const vec3_t &other) {
  x = other.x, y = other.y, z = other.z;
  return *this;
}

constexpr inline void vec3_t::operator+=(const vec3_t &other) {
  *this = *this + other;
}

constexpr inline vec3_t vec3_t::operator+(const vec3_t &other) const {
  return {x + other.x, y + other.y, z + other.z};
}

constexpr inline void vec3_t::operator-=(const vec3_t &other) {
  *this = *this - other;
}

constexpr inline vec3_t vec3_t::operator-(const vec3_t &other) const {
  return {x - other.x, y - other.y, z - other.z};
}

constexpr inline void vec3_t::operator*=(const vec3_t &other) {
  *this = *this * other;
}

constexpr inline vec3_t vec3_t::operator*(const vec3_t &other) const {
  return {x * other.x, y * other.y, z * other.z};
}

constexpr inline void vec3_t::operator*=(float k) { *this = *this * k; }

constexpr inline vec3_t vec3_t::operator*(float k) const {
  return {x * k, y * k, z * k};
}

constexpr inline void vec3_t::operator/=(const vec3_t &other) {
  *this = *this / other;
}

constexpr inline vec3_t vec3_t::operator/(const vec3_t &other) const {
  return {x / other.x, y / other.y, z / other.z};
}

constexpr inline void vec3_t::operator/=(float k) { *this = *this / k; }

constexpr inline vec3_t vec3_t::operator/(float k) const {
  return *this * (1.0f / k);
}

inline void vec3_t::normalize() { *this = normal(*this); }

struct rect_t {
	int x, y, w, h;
};

struct mat4_t {
  static mat4_t look_at(vec3_t eye, vec3_t target, vec3_t up);

  static mat4_t perspective(float fov, float width, float height, float near,
                            float far);

  static mat4_t inv(const mat4_t &mat);
  static mat4_t translation(vec3_t offset);
  static mat4_t identity();

  mat4_t() = default;
  mat4_t(float (&&args)[16]) { memcpy(v, args, sizeof(v)); }

  mat4_t(float *mat_ptr) {
    for (int i = 0; i < 16; ++i)
      v[i] = mat_ptr[i];
  }

  operator float *() { return &v[0]; }

  constexpr operator const float *() const { return &v[0]; }

  float v[16];
};

inline mat4_t operator*(const mat4_t &lhs, const mat4_t &rhs) {
  float tmp[16];
  return mat4_multiply(tmp, lhs, rhs);
}

inline mat4_t operator*(const mat4_t &m, float k) {
  float tmp[16];
  return mat4_multiply_f(tmp, m, k);
}

inline mat4_t mat4_t::identity() {
  return {{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
           0.0f, 0.0f, 0.0f, 0.0f, 1.0f}};
}

inline mat4_t mat4_t::translation(vec3_t offset) {
  mat4_t tmp{identity()};
  tmp[12] = offset.x;
  tmp[13] = offset.y;
  tmp[14] = offset.z;
  return tmp;
}

inline mat4_t mat4_t::look_at(vec3_t eye, vec3_t target, vec3_t up) {
  float tmp[16];
  return mat4_look_at(tmp, eye, target, up);
}

inline mat4_t mat4_t::inv(const mat4_t &mat) {
  float tmp[16];
  return mat4_inverse(tmp, mat);
}

inline mat4_t mat4_t::perspective(float fov, float width, float height,
                                  float near, float far) {
  float tmp[16];
  return mat4_perspective_fov(tmp, fov, width, height, near, far);
}
} // namespace nym

#endif
