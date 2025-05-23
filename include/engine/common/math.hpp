#ifndef MATH_H
#define MATH_H

#include <algorithm>
#include <cmath>

struct Math
{
	static constexpr float PI = 3.141592653f;
	static constexpr float TwoPI = 2.0f * PI;

	static float pow(float v, const uint32_t p)
	{
		float res = 1;
		for (uint32_t i{p}; i--;)
		{
			res *= v;
		}
		return res;
	}

	template<typename T>
	static T sign(T v)
	{
		return v < 0.0f ? -1.0f : 1.0f;
	}

	static float sigm(float x)
	{
		return 1.0f / (1.0f + std::exp(-x));
	}

	static float sigm_0(float x)
	{
		return sigm(x) - 0.5f;
	}

	static float radToDeg(float r)
	{
		constexpr float radian_to_deg = 180.0f / PI;
		return r * radian_to_deg;
	}

	static float clamp(float v, float min, float max)
	{
		return std::min(std::max(min, v), max);
	}

	static float gaussian(float x, float a, float b, float c)
	{
		const float n = x - b;
		return a * exp(-(n * n) / (2.0f * c * c));
	}
};

struct MathVec2
{
	template<typename TVec2>
	static float length2(TVec2 v)
	{
		return v.x * v.x + v.y * v.y;
	}

	template<typename TVec2>
	static float length(TVec2 v)
	{
		return sqrt(length2(v));
	}

	template<typename TVec2>
	static float angle(TVec2 v_1, TVec2 v_2 = {1.0f, 0.0f})
	{
		const float dot = v_1.x * v_2.x + v_1.y * v_2.y;
		const float det = v_1.x * v_2.y - v_1.y * v_2.x;
		return atan2(det, dot);
	}

	template<typename TVec2>
	static float dot(TVec2 v1, TVec2 v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	template<typename TVec2>
	static float cross(TVec2 v1, TVec2 v2)
	{
		return v1.x * v2.y - v1.y * v2.x;
	}

	template<typename Vec2Type>
	static Vec2Type normal(const Vec2Type& v)
	{
		return { -v.y, v.x };
	}

	template<typename Vec2Type>
	static Vec2Type rotateDir(const Vec2Type& v, const Vec2Type& dir)
	{
		return { dir.x * v.x - dir.y * v.y, dir.y * v.y + dir.x * v.x };
	}

	template<typename Vec2Type>
	static Vec2Type normalize(const Vec2Type& v)
	{
		return v / length(v);
	}

	template<template<typename> class Vec2Type, typename T>
	static Vec2Type<T> reflect(const Vec2Type<T>& v, const Vec2Type<T>& n)
	{
		return v - n * (MathVec2::dot(v, n) * 2.0f);
	}
};
#endif // !MATH_H
