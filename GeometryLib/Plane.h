#pragma once

#include "UtilityGlobal.h"

namespace Geom{

class Line;

class Plane
{
public:
    Plane();
	Plane(Vector3 c, Vector3 n);

	double	signedDistanceTo(Vector3 p);

	int		whichSide(Vector3 p);
	bool	onSameSide( QVector<Vector3>& pnts );

	bool	contains(Line line);

	Vector3 getProjection(Vector3 p);

public:
	Vector3 Constant, Normal;
};

}