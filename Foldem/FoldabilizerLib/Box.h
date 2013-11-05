#pragma once

#include "FoldabilizerLibGlobal.h"

struct Box
{
	Point Center;
	QVector<Vector3> Axis;
	Vector3 Extent;

	Box(){}
	Box(const Point& c, const QVector<Vector3>& axis, const Vector3& ext);
	~Box(){}

	Box &operator =(const Box &);
};


