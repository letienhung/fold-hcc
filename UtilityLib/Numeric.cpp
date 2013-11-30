#include "Numeric.h"

bool isPerp(const Vector3& v0, const Vector3& v1)
{
    return fabs(dot(v0, v1)) < ZERO_TOLERANCE_LOW;
}

double dotPerp( const Vector2& v0, const Vector2& v1 )
{
	return v0[0] * v1[1] - v0[1] * v1[0];
}

bool inRange(double t, double low, double high)
{
	return t > low  - ZERO_TOLERANCE_LOW 
		&& t < high + ZERO_TOLERANCE_LOW;
}
