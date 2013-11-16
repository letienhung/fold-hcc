#pragma once

#include "FoldabilizerLibGlobal.h"

class Line
{
public:
    Line();
	Line(Vector3 o, Vector3 d);

	Vector3 getPoint(double t);

private:
	Vector3 Origin, Direction;
};
