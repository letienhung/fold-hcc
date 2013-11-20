#include "Rectangle.h"
#include "Plane.h"
#include "Numeric.h"

Rectangle::Rectangle()
{
}

Rectangle::Rectangle( QVector<Vector3>& conners )
{
	Center = Vector3(0, 0, 0);
	foreach (Vector3 p, conners) Center += p;
	Center /= 4;

	Vector3 e0 = conners[1] - conners[0];
	Vector3 e1 = conners[3] - conners[0];

	Axis.push_back(e0.normalized());
	Axis.push_back(e1.normalized());

	Extent = Vector2(e0.norm()/2, e1.norm()/2);

	Normal = cross(e0, e1).normalized();

	Conners = conners;
}

bool Rectangle::isCoplanar( Vector3 p )
{
	Plane plane(Center, Normal);
	return plane.whichSide(p) == 0;
}

bool Rectangle::isCoplanar( Segment s )
{
	Plane plane(Center, Normal);
	return (plane.whichSide(s.P0) == 0) 
		&& (plane.whichSide(s.P1) == 0);
}

Vector2 Rectangle::getUniformCoordinates( Vector3 p )
{
	Vector3 v = p - Center;
	double x = dot(v, Axis[0])/Extent[0];
	double y = dot(v, Axis[1])/Extent[1];

	return Vector2(x, y);
}

bool Rectangle::contains( Vector3 p )
{
	Vector2 coord = this->getUniformCoordinates(p);
	return (fabs(coord[0]) <= 1 + ZERO_TOLERANCE_LOW) 
		&& (fabs(coord[1]) <= 1 + ZERO_TOLERANCE_LOW);
}

SurfaceMesh::Vector3 Rectangle::getIntersection( Segment s )
{
	Vector2 uc0 = this->getUniformCoordinates(s.P0);
	Vector2 uc1 = this->getUniformCoordinates(s.P1);

	// to do
	return Vector3(0, 0, 0);
}
