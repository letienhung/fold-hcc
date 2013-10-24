#pragma once

#include "FoldabilizerLibGlobal.h"

class Box
{
public:
	Box(){}
	Box(Point& c, QVector<Vector3>& axis, Vector3& scale);
	~Box(){}

private:
	Point Center;
	QVector<Vector3> Axis;
	Vector3 Extent;
};


