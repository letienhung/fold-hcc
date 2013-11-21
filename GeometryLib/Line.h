#pragma once

#include "UtilityGlobal.h"

namespace Goem{

class Line
{
public:
    Line();
	Line(Vector3 o, Vector3 d);

	Vector3 getPoint(double t);
	bool	contains(Vector3 p);

private:
	Vector3 Origin, Direction;
};

}