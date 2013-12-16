#include "DistLineLine.h"
#include "Numeric.h"

Geom::DistLineLine::DistLineLine(Line& l0, Line& l1)
{
	 mLine0 = &l0;
	 mLine1 = &l1;

	 compute();
}

void Geom::DistLineLine::compute()
{
	Vector3 diff = mLine0->Origin - mLine1->Origin;
	double a01 = -mLine0->Direction.dot(mLine1->Direction);
	double b0 = diff.dot(mLine0->Direction);
	double det = fabs((double)1 - a01*a01);
	double b1, s0, s1;

	if (det >= ZERO_TOLERANCE_LOW)
	{
		// Lines are not parallel.
		b1 = -diff.dot(mLine1->Direction);
		double invDet = ((double)1)/det;
		s0 = (a01*b1 - b0)*invDet;
		s1 = (a01*b0 - b1)*invDet;
	}
	else
	{
		// Lines are parallel, select any closest pair of points.
		s0 = -b0;
		s1 = (double)0;
	}

	mClosestPoint0 = mLine0->Origin + s0*mLine0->Direction;
	mClosestPoint1 = mLine1->Origin + s1*mLine1->Direction;
}

double Geom::DistLineLine::get()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.norm();
}

double Geom::DistLineLine::getSquared()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.squaredNorm();
}
