#pragma once

#include "UtilityGlobal.h"

namespace Geom{

class Line
{
public:
    Line();
	Line(Vector3 o, Vector3 d);

	double getProjTime(Vector3 p);
	Vector3 getPoint(double t);
	bool	contains(Vector3 p);

public:
	Vector3 Origin, Direction;
};

}