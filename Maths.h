#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>

#define SQ(x) ((x) * (x)) // Square macro: expands to (x)*(x). Be careful: evaluates `x` twice, may have side-effects.

template<typename T>
static T lerp(const T a, const T b, float t) {
	// Linear interpolation between `a` and `b`.
	// `t` is the interpolation factor: when t==0 returns `a`, when t==1 returns `b`.
	// Values outside [0,1] perform extrapolation.
	return a * (1.0f - t) + (b * t);
}

class Vec3 {
public:
	union {
		float v[3];
		struct { float x, y, z; }; // Convenience accessors for components.
	};

	// Default constructor: initializes vector to (0,0,0).
	Vec3() {
		x = 0;
		y = 0;
		z = 0;
	}

	// Construct from explicit components.
	Vec3(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}

	// Component-wise addition. Returns a new Vec3 = this + pVec.
	Vec3 operator+(const Vec3& pVec) const
	{
		return Vec3(v[0] + pVec.v[0], v[1] + pVec.v[1], v[2] + pVec.v[2]);
	}

	// In-place addition. Adds pVec to this vector and returns *this.
	Vec3& operator+=(const Vec3& pVec)
	{
		v[0] += pVec.v[0];
		v[1] += pVec.v[1];
		v[2] += pVec.v[2];
		return *this;
	}
	
	// subtraction
	Vec3 operator-(const Vec3& pVec) const
	{
		return Vec3(v[0] - pVec.v[0], v[1] - pVec.v[1], v[2] - pVec.v[2]);
	}

	// In-place subtraction
	Vec3& operator-=(const Vec3& pVec)
	{
		v[0] -= pVec.v[0];
		v[1] -= pVec.v[1];
		v[2] -= pVec.v[2];
		return *this;
	}

	// Component-wise multiplication (Hadamard/product). Returns a new Vec3.
	Vec3 operator*(const Vec3& pVec) const
	{
		return Vec3(v[0] * pVec.v[0], v[1] * pVec.v[1], v[2] * pVec.v[2]);
	}

	// Scalar multiplication. Returns a new Vec3 scaled by `val`.
	Vec3 operator*(const float val) const
	{
		return Vec3(v[0] * val, v[1] * val, v[2] * val);
	}

	// Division
	Vec3 operator/(const Vec3& pVec) const
	{
		return Vec3(v[0] / pVec.v[0], v[1] / pVec.v[1], v[2] / pVec.v[2]);
	}

	// Scalar division
	Vec3 operator/(const float val) const
	{
		return Vec3(v[0] / val, v[1] / val, v[2] / val);
	}

	// Unary negation. Returns a new Vec3 with each component negated.
	Vec3 operator-() const
	{
		return Vec3(-v[0], -v[1], -v[2]);
	}

	// In-place multiplication
	Vec3& operator*=(const Vec3& pVec)
	{
		v[0] *= pVec.v[0];
		v[1] *= pVec.v[1];
		v[2] *= pVec.v[2];
		return *this;
	}

	// In-place division
	Vec3& operator/=(const Vec3& pVec)
	{
		v[0] /= pVec.v[0];
		v[1] /= pVec.v[1];
		v[2] /= pVec.v[2];
		return *this;
	}

	float length() const
	{
		return sqrtf((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
	}

	float lengthsq() const
	{
		return (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
	}

	Vec3 normalise()
	{
		float len = 1.0f / sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		return Vec3(x * len, y * len, z * len);
	}

	float normalize_GetLength()
	{
		float length = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); float len = 1.0f / length;
		v[0] *= len; v[1] *= len; v[2] *= len;
		return length;
	}

	float Dot(const Vec3& pVec) const
	{
		return v[0] * pVec.v[0] + v[1] * pVec.v[1] + v[2] * pVec.v[2];
	}

	Vec3 Cross(const Vec3& v1)// aXb where a is this and b is v1 - reads a.cross(b)
	{
		return Vec3(v[1] * v1.v[2] - v[2] * v1.v[1], v[2] * v1.v[0] - v[0] * v1.v[2], v[0] * v1.v[1] - v[1] * v1.v[0]);
	}

	/*Vec3 Max(const Vec3& v1, const Vec3& v2) {
		return Vec3(std::max(v1.v[0], v2.v[0]), std::max(v1.v[1], v2.v[1]), std::max(v1.v[2], v2.v[2]));
	}

	float Max() const
	{
		return std::max(x, std::max(y, z));
	}

	Vec3 Min(const Vec3& v1, const Vec3& v2) {
		return Vec3(std::min(v1.v[0], v2.v[0]), std::min(v1.v[1], v2.v[1]), std::min(v1.v[2], v2.v[2]));
	}

	float Min() const
	{
		return std::min(x, std::min(y, z));
	}*/

	void printVec3()
	{
		std::cout << v[0] << '\t' << v[1] << '\t' << v[2] << std::endl;
	}
};

// Global function - used so often can be useful to have outside.
float Dot(const Vec3& v1, const Vec3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

class Vec4 {
public:
	union {
		float v[4];
		struct { float x, y, z, w; }; // Convenience accessors for components.
	};

	// Default constructor: initializes vector to (0,0,0).
	Vec4() {
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

	// Construct from explicit components.
	Vec4(float _x, float _y, float _z, float _w) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	// Component-wise addition. Returns a new Vec4 = this + pVec.
	Vec4 operator+(const Vec4& pVec) const
	{
		return Vec4(v[0] + pVec.v[0], v[1] + pVec.v[1], v[2] + pVec.v[2], v[3] + pVec.v[3]);
	}

	// In-place addition. Adds pVec to this vector and returns *this.
	Vec4& operator+=(const Vec4& pVec)
	{
		v[0] += pVec.v[0];
		v[1] += pVec.v[1];
		v[2] += pVec.v[2];
		v[3] += pVec.v[3];
		return *this;
	}

	// subtraction
	Vec4 operator-(const Vec4& pVec) const
	{
		return Vec4(v[0] - pVec.v[0], v[1] - pVec.v[1], v[2] - pVec.v[2], v[3] - pVec.v[3]);
	}

	// In-place subtraction
	Vec4& operator-=(const Vec4& pVec)
	{
		v[0] -= pVec.v[0];
		v[1] -= pVec.v[1];
		v[2] -= pVec.v[2];
		v[3] -= pVec.v[3];
		return *this;
	}

	// Component-wise multiplication (Hadamard/product). Returns a new Vec3.
	Vec4 operator*(const Vec4& pVec) const
	{
		return Vec4(v[0] * pVec.v[0], v[1] * pVec.v[1], v[2] * pVec.v[2], v[3] * pVec.v[3]);
	}

	// Scalar multiplication. Returns a new Vec3 scaled by `val`.
	Vec4 operator*(const float val) const
	{
		return Vec4(v[0] * val, v[1] * val, v[2] * val, v[3] * val);
	}

	// Division
	Vec4 operator/(const Vec4& pVec) const
	{
		return Vec4(v[0] / pVec.v[0], v[1] / pVec.v[1], v[2] / pVec.v[2], v[3] / pVec.v[3]);
	}

	// Scalar division
	Vec4 operator/(const float val) const
	{
		return Vec4(v[0] / val, v[1] / val, v[2] / val, v[3] / val);
	}

	// Unary negation. Returns a new Vec4 with each component negated.
	Vec4 operator-() const
	{
		return Vec4(-v[0], -v[1], -v[2], -v[3]);
	}

	// In-place multiplication
	Vec4& operator*=(const Vec4& pVec)
	{
		v[0] *= pVec.v[0];
		v[1] *= pVec.v[1];
		v[2] *= pVec.v[2];
		v[3] *= pVec.v[3];
		return *this;
	}

	// In-place division
	Vec4& operator/=(const Vec4& pVec)
	{
		v[0] /= pVec.v[0];
		v[1] /= pVec.v[1];
		v[2] /= pVec.v[2];
		v[3] /= pVec.v[3];
		return *this;
	}

	void divW() {
		x /= w;
		y /= w;
		z /= w;
		w /= w;
	}

	void printVec4()
	{
		std::cout << v[0] << '\t' << v[1] << '\t' << v[2] << '\t' << v[3] << std::endl;
	}
};

class Matrix
{
public:
	
	union
	{
		float a[4][4]; float m[16];
	};
	Matrix() {
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++) {
				if (j == i)
					a[i][j] = 1;
				else
					a[i][j] = 0;
			}
	}

	void identity()
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++) {
				if (j == i)
					a[i][j] = 1;
				else
					a[i][j] = 0;
			}
	}

	Vec4 mul(const Vec4& v)
	{
		return Vec4(
			v.x * m[0] + v.y * m[1] + v.z * m[2] + v.w * m[3],
			v.x * m[4] + v.y * m[5] + v.z * m[6] + v.w * m[7], 
			v.x * m[8] + v.y * m[9] + v.z * m[10] + v.w * m[11], 
			v.x * m[12] + v.y * m[13] + v.z * m[14] + v.w * m[15]);
	}

	Vec3 mulPoint(const Vec3& v)
	{
		return Vec3(
			(v.x * m[0] + v.y * m[1] + v.z * m[2]) + m[3],
			(v.x * m[4] + v.y * m[5] + v.z * m[6]) + m[7],
			(v.x * m[8] + v.y * m[9] + v.z * m[10]) + m[11]);
	}

	Vec3 mulVec(const Vec3& v)
	{
		return Vec3(
			(v.x * m[0] + v.y * m[1] + v.z * m[2]),
			(v.x * m[4] + v.y * m[5] + v.z * m[6]),
			(v.x * m[8] + v.y * m[9] + v.z * m[10]));
	}

	void translation(float tx, float ty, float tz)
	{
		m[3] = tx;
		m[7] = ty;
		m[11] = tz;
	}

	void rotationX(const float theta)
	{
		float sint = sin(theta);
		float cost = cos(theta);
		m[6] = cost;
		m[7] = -sint;
		m[10] = sint;
		m[11] = cost;
	}

	void rotationY(const float theta)
	{
		float sint = sin(theta);
		float cost = cos(theta);
		m[0] = cost;
		m[3] = sint;
		m[9] = -sint;
		m[11] = cost;
	}

	void rotationZ(const float theta)
	{
		float sint = sin(theta);
		float cost = cos(theta);
		m[0] = cost;
		m[1] = -sint;
		m[4] = sint;
		m[5] = cost;
	}

	void scaling(float sx, float sy, float sz)
	{
		m[0] *= sx;
		m[5] *= sy;
		m[9] *= sz;
	}

	Matrix mul(const Matrix& matrix) const
	{
		Matrix ret;
		ret.m[0] = m[0] * matrix.m[0] + m[1] * matrix.m[4] + m[2] * matrix.m[8] + m[3] * matrix.m[12];
		ret.m[1] = m[0] * matrix.m[1] + m[1] * matrix.m[5] + m[2] * matrix.m[9] + m[3] * matrix.m[13];
		ret.m[2] = m[0] * matrix.m[2] + m[1] * matrix.m[6] + m[2] * matrix.m[10] + m[3] * matrix.m[14];
		ret.m[3] = m[0] * matrix.m[3] + m[1] * matrix.m[7] + m[2] * matrix.m[11] + m[3] * matrix.m[15];
		ret.m[4] = m[4] * matrix.m[0] + m[5] * matrix.m[4] + m[6] * matrix.m[8] + m[7] * matrix.m[12];
		ret.m[5] = m[4] * matrix.m[1] + m[5] * matrix.m[5] + m[6] * matrix.m[9] + m[7] * matrix.m[13];
		ret.m[6] = m[4] * matrix.m[2] + m[5] * matrix.m[6] + m[6] * matrix.m[10] + m[7] * matrix.m[14];
		ret.m[7] = m[4] * matrix.m[3] + m[5] * matrix.m[7] + m[6] * matrix.m[11] + m[7] * matrix.m[15];
		ret.m[8] = m[8] * matrix.m[0] + m[9] * matrix.m[4] + m[10] * matrix.m[8] + m[11] * matrix.m[12];
		ret.m[9] = m[8] * matrix.m[1] + m[9] * matrix.m[5] + m[10] * matrix.m[9] + m[11] * matrix.m[13]; 
		ret.m[10] = m[8] * matrix.m[2] + m[9] * matrix.m[6] + m[10] * matrix.m[10] + m[11] * matrix.m[14]; 
		ret.m[11] = m[8] * matrix.m[3] + m[9] * matrix.m[7] + m[10] * matrix.m[11] + m[11] * matrix.m[15];
		ret.m[12] = m[12] * matrix.m[0] + m[13] * matrix.m[4] + m[14] * matrix.m[8] + m[15] * matrix.m[12];
		ret.m[13] = m[12] * matrix.m[1] + m[13] * matrix.m[5] + m[14] * matrix.m[9] + m[15] * matrix.m[13];
		ret.m[14] = m[12] * matrix.m[2] + m[13] * matrix.m[6] + m[14] * matrix.m[10] + m[15] * matrix.m[14];
		ret.m[15] = m[12] * matrix.m[3] + m[13] * matrix.m[7] + m[14] * matrix.m[11] + m[15] * matrix.m[15];
		return ret;
	}

	Matrix operator*(const Matrix& matrix)
	{
		return mul(matrix);
	}

	//implement transposing 2 matrix slide 59 -- DONE
	void transposing() 
	{
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				std::swap(a[i][j], a[j][i]);
	}

	float& operator[](int index)
	{
		return m[index];
	}

	Matrix invert()
	{
		Matrix inv;
		inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10]; 
		inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10]; 
		inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
		inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9]; 
		inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];  
		inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10]; 
		inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9]; 
		inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9]; 
		inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6]; 
		inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6]; 
		inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];  
		inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5]; 
		inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6]; 
		inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6]; 
		inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
		inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];  
		float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
		if (det == 0) {
			// Handle this case
		}
		det = 1.0 / det;
		for (int i = 0; i < 16; i++) {
			inv[i] = inv[i] * det;
		}
		return inv;
	}

	void projMat(float fov, float aspect, float _far, float _near) {
		float tfov = tan(fov / 2);
		m[0] = 1 / aspect * tfov;
		m[5] = 1 / tfov;
		m[10] = _far / (_far - _near);
		m[11] = (-_far * _near) / (_far - _near);
		m[14] = 1;
		m[15] = 0;
	}
};

// implement spherical coords class - slide 92 -- DONE

class sphericalCoords {
public:
	struct { float r; float theta; float phi; };
	sphericalCoords() {
		r = 1;
		theta = 0;
		phi = 0;
	}
	sphericalCoords(float _r, float _theta, float _phi) {
		r = _r;
		theta = _theta;
		phi = _phi;
	}

	// flip y/z when calling function to get shading to camera
	float calcTheta(Vec3& v)
	{
		theta = acosf(v.z / r);
		return theta;
	}

	float calcPhi(const Vec3& v)
	{
		phi = atan2f(v.y, v.x);
		return phi;
	}

	Vec3 toCartesian() const
	{
		float sinTheta = sinf(theta);
		float x = r * sinTheta * cosf(phi);
		float y = r * sinTheta * sinf(phi);
		float z = r * cosf(theta);
		return Vec3(x, y, z);
	}

};

class Quaternion {
public:
	union{
		struct { float a; float b; float c; float d; };
		float q[4]; };

	Quaternion() {
		a = 0;
		b = 0;
		c = 0;
		d = 0;
	}
	Quaternion(float _a, float _b, float _c, float _d) {
		a = _a;
		b = _b;
		c = _c;
		d = _d;
	}

	Matrix toMatrix() {
		float aa = a * a, ab = a * b, ac = a * c;
		float bb = b * b, bc = b * c, cc = c * c;
		float da = d * a, db = d * b, dc = d * c;
		Matrix m;
		m[0] = 1 - 2 * (bb + cc); m[1] = 2 * (ab - dc); m[2] = 2 * (ac + db); m[3] = 0;
		m[4] = 2 * (ab + dc); m[5] = 1 - 2 * (aa + cc); m[6] = 2 * (bc - da); m[7] = 0;
		m[8] = 2 * (ac - db); m[9] = 2 * (bc + da); m[10] = 1 - 2 * (aa + bb); m[11] = 0;
		m[12] = m[13] = m[14] = 0; m[15] = 1;
		return m;
	}

	float magnitude() {
		return sqrtf(SQ(a) + SQ(b) + SQ(c) + SQ(d));
	}
	void normalization() {
		float m = magnitude();
		if (m == 0) return;
		a /= m;
		b /= m;
		c /= m;
		d /= m;
	}
	Quaternion conjugate() const{
		return Quaternion(-a, -b, -c, d);
	}
	void inverse() {
		float m = magnitude();
		Quaternion conj = conjugate();
		a = conj.a / m;
		b = conj.b / m;
		c = conj.c / m;
		d = conj.d / m;
	}
	Quaternion operator-() {
		return Quaternion(-a, -b, -c, -d);
	}
	Quaternion operator*(const Quaternion& q) const
	{
		float a1 = a, b1 = b, c1 = c, d1 = d;
		float a2 = q.a, b2 = q.b, c2 = q.c, d2 = q.d;

		Quaternion result;

		result.a = d1 * d2 - a1 * a2 - b1 * b2 - c1 * c2;
		result.b = d1 * a2 + a1 * d2 + b1 * c2 - c1 * b2;
		result.c = d1 * b2 - a1 * c2 + b1 * d2 + c1 * a2;
		result.d = d1 * c2 + a1 * b2 - b1 * a2 + c1 * d2;

		return result;
	}
	void axisAngle(const Vec3& vec, float angleRadians) {
		// Assumes axis is normalized.
		float halfAngle = angleRadians * 0.5f;
		float s = std::sin(halfAngle);
		float cAngle = std::cos(halfAngle);

		// Set quaternion components.
		a = vec.x * s;
		b = vec.y * s;
		c = vec.z * s;
		d = cAngle;
	}

	Quaternion slerpAngle(Quaternion& q2, float t) {
		float dot = d * q2.d + a * q2.a + b * q2.b + c * q2.c;
		float theta;
		if (dot < 0.0f) {
			q2 = -q2;
			theta = acosf(-dot);
		}
		else {
			theta = acosf(dot);
		}
		
		float sint = sin(theta);
		float sint1 = sin(theta * (1 - t));
		float sint2 = sin(theta * t);

		float w1 = sint1 / sint;
		float w2 = sint2 / sint;

		return Quaternion(
			(w1 * a + w2 * q2.a)
			, (w1 * b + w2 * q2.b)
			, (w1 * c + w2 * q2.c)
			, (w1 * d + w2 * q2.d));
	}

	Vec3 rotatePoint(const Quaternion& q, Vec3& vec) {
		Quaternion pq(0.0f, vec.x, vec.y, vec.z);
		Quaternion conj = q.conjugate();
		Quaternion pqi = q * pq * conj;
		Vec3 rotated(pqi.a, pqi.b, pqi.a);
		return rotated;
	}

	//slide 108 implement shortest path rotation
};

class shadingFrame {
public:
	Vec3 n;
	Vec3 u;
	Vec3 v;
	Matrix F;

	shadingFrame() 
		: n(0.0f, 0.0f, 0.0f)
		, u(0.0f, 0.0f, 0.0f)
		, v(0.0f, 0.0f, 0.0f)
		, F() {}

	Matrix& buildBasis() {
		F.a[0][0] = u.x; F.a[0][1] = v.x; F.a[0][2] = n.x; F.a[0][3] = 0.0f;
		F.a[1][0] = u.y; F.a[1][1] = v.y; F.a[1][2] = n.y; F.a[1][3] = 0.0f;
		F.a[2][0] = u.z; F.a[2][1] = v.z; F.a[2][2] = n.z; F.a[2][3] = 0.0f;
		F.a[3][0] = 0.0f; F.a[3][1] = 0.0f; F.a[3][2] = 0.0f; F.a[3][3] = 1.0f;
		return F;
	}
	Matrix inverse() {
		Matrix inv = F;
		inv.transposing();
		return inv;
	}
	/*Vec3 shadingToWorldVec(const Vec3& s) {
		buildBasis();
		return F.mulVec(s);
	}
	Vec3 worldToShadingVec(const Vec3& w) {
		buildBasis();
		Matrix inv = F.transposing();
		return inv.mulVec(w);
	}*/

	void GS0(Vec3& vec) {
		n = vec.normalise();
		Vec3 uP;
		if (n.x < 0.99f && n.x > -0.99f)
			uP = Vec3(1.0f, 0.0f, 0.0f);
		else
			uP = Vec3(0.0f, 1.0f, 0.0f);
		u = (n.Cross(uP)).normalise();
		v = (n.Cross(u)).normalise(); // Normalizing due to other vectors possibly not normalizing properly
		buildBasis();
	}
};

class Colour {
public:
	union {
		struct { float r; float g; float b; float a; };
		float c[4];
	};

	Colour() {
		r = 0.0f;
		g = 0.0f;
		b = 0.0f;
		a = 0.0f;
	}
	Colour(float _r, float _g, float _b, float _a) {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	Colour(unsigned char R, unsigned char G, unsigned char B, unsigned A) {
		const float str = 1.0f / 255.0f;
		r = R * str;
		g = G * str;
		b = B * str;
		a = A * str;

	}

	Colour operator+(const Colour& colour) const {
		return Colour(c[0] + colour.c[0], c[1] + colour.c[1], c[2] + colour.c[2], c[3] + colour.c[3]);
	}
	Colour operator*(const Colour& colour) const {
		return Colour(c[0] * colour.c[0], c[1] * colour.c[1], c[2] * colour.c[2], c[3] * colour.c[3]);
	}
	Colour operator*(const float a) const {
		return Colour(c[0] * a, c[1] * a, c[2] * a, c[3] * a);
	}
	Colour operator/(const float a) const {
		return Colour(c[0] / a, c[1] / a, c[2] / a, c[3] / a);
	}
};

//implement colour class 113 -- DONE
// do slide 116 -- DONEISH