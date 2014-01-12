#pragma once

#include "UtilityGlobal.h"

namespace Geom {

class Segment2
{
public:
	Segment2();
	Segment2(const Vector2& p0, const Vector2& p1);

	QVector<Vector2> getUniformSamples(int N);

public:
	// End-point representation.
	Vector2 P0, P1;

	// Center-direction-extent representation.
	Vector2 Center;
	Vector2 Direction;
	double Extent;
};

}
