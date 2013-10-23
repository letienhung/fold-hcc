#pragma once

#include "FoldabilizerLibGlobal.h"

class Box
{
public:
	Box(){}
	Box(Point& c, QVector<Vector3>& axis, Vector3& scale);
	~Box(){}

private:
	Point mCenter;
	QVector<Vector3> mAxis;
	Vector3 mScale;
};


