#pragma once
#include "../include.hpp"

class Vector2
{
public:
	float x = 0;
	float y = 0;

	Vector2() = default;
	Vector2(float x, float y) : x(x), y(y) {};

	Vector2 operator+(const Vector2& other) const {
		return Vector2(x + other.x, y + other.y);
	}
	Vector2 operator-(const Vector2& other) const {
		return Vector2(x - other.x, y - other.y);
	}
	Vector2 operator*(const Vector2& other) const {
		return Vector2(x * other.x, y * other.y);
	}
	Vector2 operator/(const Vector2& other) const {
		return Vector2(x / other.x, y / other.y);
	}

	float distance(const Vector2& other) const {
		return std::sqrtf(std::powf(other.x - x, 2) + std::powf(other.y - y, 2));
	}
	std::string to_string() {
		return std::to_string(x) + " " + std::to_string(y);
	}

	ImVec2 to_imvec2() {
		return ImVec2(x, y);
	}
};

class Vector3
{
public:
	float x = 0;
	float y = 0;
	float z = 0;

	Vector3() = default;
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {};

	Vector3 operator+(const Vector3& other) const {
		return Vector3(x + other.x, y + other.y, z + other.z);
	}
	Vector3 operator-(const Vector3& other) const {
		return Vector3(x - other.x, y - other.y, z - other.z);
	}
	Vector3 operator*(const Vector3& other) const {
		return Vector3(x * other.x, y * other.y, z * other.z);
	}
	Vector3 operator/(const Vector3& other) const {
		return Vector3(x / other.x, y / other.y, z / other.z);
	}

	float Distance(const Vector3& other) const {
		return std::sqrtf(std::powf(other.x - x, 2) + std::powf(other.y - y, 2) + std::powf(other.z - z, 2));
	}

	bool IsZero() {
		return x == 0 && y == 0 && z == 0;
	}

	float Dot(const Vector3& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	std::string to_string() {
		return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
	}
};

class Vector4
{
public:
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 0;

	Vector4() = default;
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};

	Vector4 operator+(const Vector4& other) const {
		return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
	}
	Vector4 operator-(const Vector4& other) const {
		return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
	}
	Vector4 operator*(const Vector4& other) const {
		return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
	}
	Vector4 operator/(const Vector4& other) const {
		return Vector4(x / other.x, y / other.y, z / other.z, w / other.w);
	}

	std::string to_string() {
		return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + " " + std::to_string(w);
	}
};

struct Matrix3x4 {
	Vector4 vec0;
	Vector4 vec1;
	Vector4 vec2;
};

struct Matrix4x4 {
	union {
		struct {
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		}; float m[4][4];
	};
};

template <typename _T>
inline static unsigned int constexpr hash_string(_T const* input)
{
	unsigned int hash = 0;
	while (*input)
	{
		hash = (hash << 5) + hash + *input++;
	}
	return hash;
}

class Math
{
public:
	static inline ImVec2 screen_size = ImVec2(static_cast<float>(GetSystemMetrics(SM_CXSCREEN)), static_cast<float>(GetSystemMetrics(SM_CYSCREEN)));
	static inline ImVec2 screen_center = ImVec2(screen_size.x / 2, screen_size.y / 2);

	static inline bool WorldToScreen(Vector3 EntityPos, Vector2& ScreenPos, Matrix4x4 ViewMatrix)
	{
		Vector3 TransVec = Vector3(ViewMatrix._14, ViewMatrix._24, ViewMatrix._34);
		Vector3 RightVec = Vector3(ViewMatrix._11, ViewMatrix._21, ViewMatrix._31);
		Vector3 UpVec = Vector3(ViewMatrix._12, ViewMatrix._22, ViewMatrix._32);

		float w = TransVec.Dot(EntityPos) + ViewMatrix._44;
		if (w < 0.098f) 
			return false;

		float y = UpVec.Dot(EntityPos) + ViewMatrix._42;
		float x = RightVec.Dot(EntityPos) + ViewMatrix._41;

		ScreenPos = Vector2((screen_size.x / 2) * (1.f + x / w), (screen_size.y / 2) * (1.f - y / w));
		if (ScreenPos.x > screen_size.x || ScreenPos.y > screen_size.y || ScreenPos.x < 0 || ScreenPos.y < 0)
			return false;

		return true;
	}
};