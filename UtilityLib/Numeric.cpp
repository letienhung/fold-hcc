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

Vector3 perpVector( const Vector3& n )
{
	if ((abs(n.y()) >= 0.9 * abs(n.x())) && abs(n.z()) >= 0.9 * abs(n.x())) 
	{
		return Vec3d(0.0, -n.z(), n.y());
	}
	else if ( abs(n.x()) >= 0.9 * abs(n.y()) && abs(n.z()) >= 0.9 * abs(n.y()) ) 
	{
		return Vec3d(-n.z(), 0.0, n.x());
	}
	else 
	{
		return Vec3d(-n.y(), n.x(), 0.0);
	}
}

double maxDouble()
{
	return std::numeric_limits<double>::max();
}

double minDouble()
{
	return - maxDouble();
}

SurfaceMesh::Vector3 minimize( const Vector3 a, const Vector3 b )
{
	Vector3 c = a;
	for (int i = 0; i < 3; i++)	
		if (b[i] < a[i]) c[i] = b[i];

	return c;
}

SurfaceMesh::Vector3 maximize( const Vector3 a, const Vector3 b )
{
	Vector3 c = a;
	for (int i = 0; i < 3; i++)	
		if (b[i] > a[i]) c[i] = b[i];

	return c;
}

QVector<Vector3> XYZ()
{
	QVector<Vector3> a;
	a.push_back(Vector3(1, 0, 0));
	a.push_back(Vector3(0, 1, 0));
	a.push_back(Vector3(0, 0, 1));
	return a;
}

double periodicalRanged( double a, double b, double v )
{
	double p = b -a;
	double n = (v - a) / p;
	if(n < 0)
		return v - p * int(n) + p;
	else
		return v - p * int(n);
}

double radians2degrees( double r )
{
	return 180.0 * r / M_PI;
}

double degrees2radians( double a )
{
	return M_PI * a / 180;
}

bool areCollinear( const Vector3& v0, const Vector3& v1 )
{
	return cross(v0, v1).norm() < ZERO_TOLERANCE_LOW;
}

bool solveQuadratic( double a, double b, double c, double& root1, double &root2 )
{
	double d2 = b*b - 4*a*c;
	if (d2 < 0 ) return false;

	double d = sqrt(d2);
	root1 = (-b + d) / (2 * a);
	root2 = (-b - d) / (2 * a);
	return true;
}