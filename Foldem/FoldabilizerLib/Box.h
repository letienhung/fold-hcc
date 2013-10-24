#pragma once

#include "FoldabilizerLibGlobal.h"

class Box
{
public:
	Box(){}
	Box(Point& c, QVector<Vector3>& axis, Vector3& ext);
	~Box(){}

	Box &operator =(const Box &);

private:
	Point mCenter;
	QVector<Vector3> mAxis;
	Vector3 mExtent;
};


