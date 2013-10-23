#include "Box.h"

Box::Box(Point& c, QVector<Vector3>& axis, Vector3& scale)
{
	mCenter = c;
	mAxis = axis;
	mScale = scale;
}
