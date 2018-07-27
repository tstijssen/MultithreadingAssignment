#include "TypeDefs.h"

using namespace std;
bool g_DETAILED_TIMING = false;
bool g_SPHERE_DEATH = false;

// Return unit length vector in the same direction as given one
Vector2f Normalise(const Vector2f& v)
{
	float lengthSq = v.x*v.x + v.y*v.y;

	// Ensure vector is not zero length (use BaseMath.h float approx. fn with default epsilon)
	if (IsZero(lengthSq))
	{
		return { 0.0f, 0.0f };
	}
	else
	{
		float invLength = InvSqrt(lengthSq);
		return { v.x * invLength, v.y * invLength };
	}
}

// Return unit length vector in the same direction as given one
Vector3f Normalise(const Vector3f& v)
{
	float lengthSq = v.x*v.x + v.y*v.y + v.z * v.z;

	// Ensure vector is not zero length (use BaseMath.h float approx. fn with default epsilon)
	if (IsZero(lengthSq))
	{
		return { 0.0f, 0.0f, 0.0f };
	}
	else
	{
		float invLength = InvSqrt(lengthSq);
		return { v.x * invLength, v.y * invLength, v.z * invLength };
	}
}