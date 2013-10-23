#pragma once

#include "FoldabilizerLibGlobal.h"

class Box
{
public:
    Box();

private:
	Point center;
	QVector<Vector3> axis;
	Vector3 scale;
};


