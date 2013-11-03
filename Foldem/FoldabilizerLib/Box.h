#pragma once

#include "FoldabilizerLibGlobal.h"

class Box
{
public:
	Box(){}
	Box(Point& c, QVector<Vector3>& axis, Vector3& ext);
	~Box(){}

	Box &operator =(const Box &);

public:
	Point Center;
	QVector<Vector3> Axis;
	Vector3 Extent;
};


