#include "DistPointRect.h"


Geom::DistPointRect::DistPointRect( Vector3& point, Rectangle& rect )
{
	mPoint = &point;
	mRectangle = &rect;

	compute();
}

void Geom::DistPointRect::compute()
{
	Vector3 diff = mRectangle->Center - *mPoint;
	double b0 = diff.dot(mRectangle->Axis[0]);
	double b1 = diff.dot(mRectangle->Axis[1]);
	double s0 = -b0, s1 = -b1;

	if (s0 < -mRectangle->Extent[0])
	{
		s0 = -mRectangle->Extent[0];
	}
	else if (s0 > mRectangle->Extent[0])
	{
		s0 = mRectangle->Extent[0];
	}

	if (s1 < -mRectangle->Extent[1])
	{
		s1 = -mRectangle->Extent[1];
	}
	else if (s1 > mRectangle->Extent[1])
	{
		s1 = mRectangle->Extent[1];
	}

	mClosestPoint0 = *mPoint;
	mClosestPoint1 = mRectangle->Center + s0*mRectangle->Axis[0] +
		s1*mRectangle->Axis[1];

	mRectCoord[0] = s0;
	mRectCoord[1] = s1;
}


double Geom::DistPointRect::get()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.norm();
}

double Geom::DistPointRect::getSquared()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.squaredNorm();
}