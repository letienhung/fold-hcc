#include "Box.h"

Box::Box(Point& c, QVector<Vector3>& axis, Vector3& ext)
{
	Center = c;
	Axis = axis;
	Extent = ext;
}

Box &Box::operator =(const Box &b)
{
	Center = b.Center;
	Axis = b.Axis;
	Extent = b.Extent;

    return *this;
}

