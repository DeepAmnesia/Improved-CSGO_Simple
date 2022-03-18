#pragma once

#include "../sdk.hpp"

#include <DirectXMath.h>
#include <map>
#define RAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define DEG2RAD(x) DirectX::XMConvertToRadians(x)
#define M_PI 3.14159265358979323846
#define PI_F	((float)(M_PI)) 

#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TICKS_TO_TIME(t) (g_GlobalVars->interval_per_tick * (t) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
namespace Math
{
	void MovementFix(CUserCmd* m_Cmd, QAngle wish_angle, QAngle old_angles);
	inline float FASTSQRT(float x)
	{
		unsigned int i = *(unsigned int*)&x;

		i += 127 << 23;
		i >>= 1;
		return *(float*)&i;
	}
	__forceinline float NormalizeAngle(float flAngle)
	{
		flAngle = fmod(flAngle, 360.0f);
		if (flAngle > 180.0f)
			flAngle -= 360.0f;
		if (flAngle < -180.0f)
			flAngle += 360.0f;

		return flAngle;
	}
	float VectorDistance(const Vector& v1, const Vector& v2);
	QAngle CalcAngle(const Vector& src, const Vector& dst);
	float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle);
	template<class T>
	void Normalize3(T& vec)
	{
		for (auto i = 0; i < 2; i++) {
			while (vec[i] < -180.0f) vec[i] += 360.0f;
			while (vec[i] >  180.0f) vec[i] -= 360.0f;
		}
		vec[2] = 0.f;
	}
    void ClampAngles(QAngle& angles);
    void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
    void AngleVectors(const QAngle &angles, Vector& forward);
    void AngleVectors(const QAngle &angles, Vector& forward, Vector& right, Vector& up);
    void VectorAngles(const Vector& forward, QAngle& angles);
    bool WorldToScreen(const Vector& in, Vector& out);
	float NormalizeYaw(float value);
	template<class T, class U>
	static T clamp(const T& in, const U& low, const U& high)
	{
		if (in <= low)
			return low;

		if (in >= high)
			return high;

		return in;
	}
	void FixAngles(QAngle& angles);
	float RandomFloat(float min, float max);
	void RandomSeed(uint32_t seed);
	template <typename T>
	static T Interpolate(const T& t1, const T& t2, float progress)
	{
		if (t1 == t2)
			return t1;

		return t2 * progress + t1 * (1.0f - progress);
	}
}