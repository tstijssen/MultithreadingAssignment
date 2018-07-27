#pragma once
//#include "ImportedFunctions.h"
#include <math.h>
#include <stdlib.h>
#include <list>
#include <math.h>
#include <vector>
#include <chrono>
#include <ctime>

const int kNumOfSpheres = 2000;
const int kNumOfStatics = kNumOfSpheres / 2;
const int kNumOfReports = 10;
const int kPartSize = 2;		// size of each partition array (array is [][][])
const int kDamage = 20;

extern bool g_DETAILED_TIMING;
extern bool g_SPHERE_DEATH;
enum SphereStatus { Inactive, Destroyed, Active, Checked };

const float kUpperBound = 1000.0f;
const float kLowerBound = -1000.0f;
const float kMaxSphereSize = 1.0f;
const float kMinSphereSize = 0.5f;
const float kSphereModelSize = 10.0f;
const float kPartitionSize = 500.0f;
const float kHidePartFloor = -2000.0f;

const float kMinXVelocity = -5.0f;
const float kMinYVelocity = -5.0f;
const float kMaxXVelocity = 5.0f;
const float kMaxYVelocity = 5.0f;

const float kMaxSpeed = 100.0f;
const float kMinSpeed = 0.0f;
const float kNormalSpeed = 20.0f;
const float kCameraSpeed = 250.0f;
const float kMouseSpeed = 100.0f;

const float kPlayerSpeed = 0.01f;

const float kfEpsilon = 0.5e-6f;    // For 32-bit floats

struct Int2
{
	int first, last;
};

struct Vector3f
{
	float x, y, z;

	Vector3f operator+(const Vector3f& rhs)
	{
		return Vector3f{ x + rhs.x, y + rhs.y, z + rhs.z };
	}

	Vector3f operator-(const Vector3f& rhs)
	{
		return Vector3f{ x - rhs.x, y - rhs.y, z - rhs.z };
	}

	Vector3f operator*(const float  s)
	{
		return Vector3f{ x * s, y * s, z * s };
	}
};

inline Vector3f operator*(const float s, const Vector3f& rhs)
{
	return Vector3f{ rhs.x * s, rhs.y * s, rhs.z * s };
}


struct Vector2f
{
	float x, y;

	Vector2f operator+(const Vector2f& rhs)
	{
		return Vector2f{ x + rhs.x, y + rhs.y };
	}

	Vector2f operator-(const Vector2f& rhs)
	{
		return Vector2f{ x - rhs.x, y - rhs.y };
	}

	Vector2f operator*(const float  s)
	{
		return Vector2f{ x * s, y * s };
	}
};

inline Vector2f operator*(const float s, const Vector2f& rhs)
{
	return Vector2f{ rhs.x * s, rhs.y * s };
}

struct Colour3f
{
	float r, g, b;
};

struct sBallCollisionData
{
	int m_Health = 10;
	std::string m_Name;
};

struct sPartitionData
{
	Vector3f pos;
	float distToEdge;
	float ySize;
};

struct sBallModelData
{
	std::string m_Name;
	Colour3f m_Colour;
	int m_Health = 10;
};

// Return a random number in the range between rangeMin and rangeMax inclusive
// range_min <= random number <= range_max
inline float random(float rangeMin, float rangeMax)
{
	float result = (float)rand() / (float)(RAND_MAX + 1);
	result *= (float)(rangeMax - rangeMin);
	result += rangeMin;

	return result;
}

inline float dot(const Vector2f& a, const Vector2f& b)
{
	return a.x * b.x + a.y * b.y;
}

inline float dot(const Vector3f& a, const Vector3f& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline bool IsZero(const float x, const float fEpsilon = kfEpsilon)
{
	return abs((long)x) < fEpsilon;
}

inline float InvSqrt(const float x)
{
	return 1.0f / sqrt(x);
}

// Return unit length vector in the same direction as given one
Vector2f Normalise(const Vector2f& v);

Vector3f Normalise(const Vector3f& v);