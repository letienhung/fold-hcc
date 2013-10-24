#include "Box.h"

Box::Box(Point& c, QVector<Vector3>& axis, Vector3& ext)
{
	mCenter = c;
	mAxis = axis;
	mExtent = ext;
}

Box &Box::operator =(const Box &b)
{
	mCenter = b.mCenter;
	mAxis = b.mAxis;
	mExtent = b.mExtent;

    return *this;
}

