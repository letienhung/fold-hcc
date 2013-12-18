#include "Plane.h"
#include "Numeric.h"
#include "Line.h"

Geom::Plane::Plane()
{
	Constant = Vector3(0,0,0);
	Normal = Vector3(0,0,1);
}

Geom::Plane::Plane( Vector3 c, Vector3 n )
	:Constant(c), Normal(n)
{
	Normal.normalize();
}

double Geom::Plane::signedDistanceTo( Vector3 p )
{
	Vector3 dv = p - Constant;
	return dot(dv, Normal);
}

int Geom::Plane::whichSide(Vector3 p)
{
	double sd = signedDistanceTo(p);
	if (sd > ZERO_TOLERANCE_LOW)
		return 1;
	else if(sd < -ZERO_TOLERANCE_LOW)
		return -1;
	else 
		return 0;
}

bool Geom::Plane::onSameSide( QVector<Vector3>& pnts )
{
	QVector<int> sides;
	foreach(Vector3 p, pnts) sides.push_back(whichSide(p));

	bool areSame = true;
	for (int i = 0; i < sides.size()-1; i++)
	{
		if (sides[i] != sides[i+1])
		{
			areSame = false;
			break;
		}
	}

	return areSame;
}

bool Geom::Plane::contains( Line line )
{
	return this->whichSide(line.getPoint(0)) == 0 
		&& this->whichSide(line.getPoint(1)) == 0;
}

SurfaceMesh::Vector3 Geom::Plane::getProjection( Vector3 p )
{
	double dis = signedDistanceTo(p);
	return p - dis * Normal;
}
