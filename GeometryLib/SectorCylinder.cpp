#include "SectorCylinder.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "Plane.h"
#include "Numeric.h"

Geom::SectorCylinder::SectorCylinder( Segment a, Segment r1, Vector3 v2 )
{
	Origin = a.P0;
	Axis = a.Direction;
	V1 = r1.Direction;
	V2 = v2.normalized();
	Height = a.length();
	Radius = r1.length();

	// make right handed
	Vector3 crossV1V2 = cross(V1, V2);
	if (dot(crossV1V2, Axis) < 0)
	{
		Vector3 v = V1;
		V1 = V2;
		V2 = v;
	}
}

bool Geom::SectorCylinder::intersects( Segment& seg )
{
	// closest dist seg
	Segment axisSeg = getAxisSegment();
	DistSegSeg dss(axisSeg, seg);
	Vector3 P = dss.mClosestPoint0;
	double t = axisSeg.getProjCoordinates(P);

	// no intersection: beyond bottom or top
	if (fabs(t) > 1.0 - ZERO_TOLERANCE_LOW)	
		return false;
	// P0, P1 lie on one cross section
	// compare dist to radius
	else
		return (dss.get() <= Radius);
}

bool Geom::SectorCylinder::intersects( Rectangle& rect )
{
	// closest dist seg
	Segment axisSeg = getAxisSegment();
	DistSegRect dsr(axisSeg, rect);
	Vector3 P = dsr.mClosestPoint0;
	double t = axisSeg.getProjCoordinates(P);

	// no intersection: beyond bottom or top
	if (fabs(t) > 1.0 - ZERO_TOLERANCE_LOW)
		return false;
	// P0, P1 lie on one cross section
	// compare dist to radius
	else
		return (dsr.get() <= Radius);
}

Geom::Segment Geom::SectorCylinder::getAxisSegment()
{
	return Segment(Origin, Origin + Height * Axis);
}
