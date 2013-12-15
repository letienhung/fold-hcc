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

double invSqrt (double value)
{
	if (value != (double)0)
		return ((double)1)/sqrt(value);
	else
		return (double)0;
}

void generateComplementBasis( Vector3& u, Vector3& v, const Vector3& w )
{
	double invLength;

	if (fabs(w[0]) >= fabs(w[1]))
	{
		// W.x or W.z is the largest magnitude component, swap them
		invLength = invSqrt(w[0]*w[0] +	w[2]*w[2]);
		u[0] = -w[2]*invLength;
		u[1] = (double)0;
		u[2] = +w[0]*invLength;
		v[0] = w[1]*u[2];
		v[1] = w[2]*u[0] - w[0]*u[2];
		v[2] = -w[1]*u[0];
	}
	else
	{
		// W.y or W.z is the largest magnitude component, swap them
		invLength = invSqrt(w[1]*w[1] +	w[2]*w[2]);
		u[0] = (double)0;
		u[1] = +w[2]*invLength;
		u[2] = -w[1]*invLength;
		v[0] = w[1]*u[2] - w[2]*u[1];
		v[1] = -w[0]*u[2];
		v[2] = w[0]*u[1];
	}
}
