#include "SectorCylinder.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "Plane.h"
#include "Numeric.h"

Geom::SectorCylinder::SectorCylinder()
{
	Origin = Vector3(0,0,0);
	Axis = Vector3(0,0,-1);
	V1 = Vector3(0,1,0);
	V2 = Vector3(1,0,0);
	Height = 1;
	Radius = 1;
	Phi = 90;
}

Geom::SectorCylinder::SectorCylinder( Segment a, Segment r1, Vector3 v2 )
{
	Origin = a.P0;
	Axis = a.Direction;
	V1 = r1.Direction;
	V2 = v2.normalized();
	Height = a.length();
	Radius = r1.length();
	double dotProd = dot(V1, V2);
	Phi = radians2degrees(acos(RANGED(-1, dotProd, 1)));

	// make right handed
	Vector3 crossV1V2 = cross(V1, V2);
	if (dot(crossV1V2, Axis) < 0)
	{
		std::swap(V1, V2);
	}
}

Geom::SectorCylinder::SectorCylinder(Vector3 o, Vector3 x, Vector3 y, Vector3 z, double h, double r)
{
	Origin = o;
	V1 = x;
	V2 = y;
	Axis = z;
	Height = h;
	Radius = r;
}

bool Geom::SectorCylinder::intersects( Segment& seg )
{
	Segment axisSeg = getAxisSegment();
	DistSegSeg dss(axisSeg, seg);

	return contains(dss.mClosestPoint1);
}

bool Geom::SectorCylinder::intersects( Rectangle& rect )
{
	Segment axisSeg = getAxisSegment();
	DistSegRect dsr(axisSeg, rect);

	return contains(dsr.mClosestPoint1);
}

Geom::Segment Geom::SectorCylinder::getAxisSegment()
{
	return Segment(Origin, Origin + Height * Axis);
}

QStringList Geom::SectorCylinder::toStrList()
{
	return QStringList() << "SectorCylinder: "
	 << "Origin = " + qStr(Origin)
	 << "Axis = " + qStr(Axis)
	 << "V1 = " + qStr(V1)
	 << "V2 = " + qStr(V2)
	 << "Height = " + QString::number(Height)
	 << "Radius = " + QString::number(Radius)
	 << "Phi = " + QString::number(Phi);
}

// cylindrical coordinates normalized by extents
Vector3 Geom::SectorCylinder::getCoordinates( Vector3 p )
{
	Vector3 O2P = p - Origin;
	double z = dot(Axis, O2P) / Height;

	Vector3 projP = Origin + z * Height * Axis;	
	Vector3 projP2P = p - projP;
	double rho = projP2P.norm() / Radius;

	double dotProd = dot(V1, projP2P.normalized());
	double angle = acos(RANGED(-1, dotProd, 1));
	Vector3 crossV1ProjP2P = cross(V1, projP2P);
	if (dot(crossV1ProjP2P, Axis) < 0) angle *= -1;
	double phi = radians2degrees(angle) / Phi;

	return Vector3(rho, phi, z);
}

Vector3 Geom::SectorCylinder::getPosition( Vector3 coord )
{
	double z = coord.z();
	Vector3 up = Origin + z * Height * Axis;

	double rho = coord.x() * Radius;
	double phi = degrees2radians(coord.y() * Phi);
	double a = rho * cos(phi);
	double b = rho * sin(phi);

	Vector3 va = a * V1;
	Vector3 vb = b * cross(Axis, V1);

	return up + va + vb;
}

bool Geom::SectorCylinder::contains( Vector3 p )
{
	Vector3 coord = getCoordinates(p);
	return  coord[0] < 1 + ZERO_TOLERANCE_LOW // rho
		&& (coord[1] > -ZERO_TOLERANCE_LOW && coord[1] < 1 + ZERO_TOLERANCE_LOW)  // phi
		&& (coord[2] > -ZERO_TOLERANCE_LOW && coord[2] < 1 + ZERO_TOLERANCE_LOW); // z
}

void Geom::SectorCylinder::translate( Vector3 t )
{
	Origin += t;
}

void Geom::SectorCylinder::shrinkEpsilon()
{
	double delta = ZERO_TOLERANCE_LOW * Radius;
	Origin += delta * V1 + delta * V2 + delta * Axis;

	double scale = 1 - 2 * ZERO_TOLERANCE_LOW;
	Radius *= scale;
	Height *= scale;
}

QVector<Vector3> Geom::SectorCylinder::getConners()
{
	return QVector<Vector3>()
		<< Origin 
		<< Origin + Height * Axis
		<< Origin + Radius * V1
		<< Origin + Radius * V1 + Height * Axis
		<< Origin + Radius * V1
		<< Origin + Radius * V1 + Height * Axis;
}
