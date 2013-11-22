#pragma once

#include "UtilityGlobal.h"

namespace Geom{

class Plane
{
public:
    Plane();
	Plane(Vector3 c, Vector3 n);

	double	signedDistanceTo(Vector3 p);
	int		whichSide(Vector3 p);
	bool	onSameSide( QVector<Vector3>& pnts );
private:
	Vector3 Constant, Normal;
};

}