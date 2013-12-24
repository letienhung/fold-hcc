#include "SectorCylinder.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"

Geom::SectorCylinder::SectorCylinder( Segment a, Segment r1, Vector3 v2 )
{
	Origin = a.P0;
	Axis = a.Direction;
	V1 = r1.Direction;
	V2 = v2.normalized();
	Height = a.length();
	Radius = r1.length();

	A = a;
	R1 = r1;
	R2 = Segment(Origin, Origin + Radius*V2);

	// make right handed

}

bool Geom::SectorCylinder::intersects( Segment& seg )
{
	return false;
}

bool Geom::SectorCylinder::intersects( Rectangle& rect )
{
	return false;
}
