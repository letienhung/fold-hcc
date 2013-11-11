#include "Box.h"

Box::Box(const Point& c, const QVector<Vector3>& axis, const Vector3& ext)
{
	Center = c;
	Axis = axis;
	Extent = ext;
}

Box &Box::operator =(const Box &b)
{
	Center = b.Center;
	Axis = b.Axis;
	Extent = b.Extent;

    return *this;
}

Vector3 Box::getCoordinates( Vector3 p )
{
	Vector3 coord;
	p = p - Center;
	for (int i = 0; i < 3; i++)
		coord[i] = dot(p, Axis[i]);

	return coord;
}

SurfaceMesh::Vector3 Box::getPosition( Vector3 coord )
{
	Vector3 pos = Center;
	for (int i = 0; i < 3; i++)
		pos += coord[i] * Axis[i];

	return pos;
}

SurfaceMesh::Vector3 Box::getUniformCoordinates( Vector3 p )
{
	Vector3 coord;
	p = p - Center;
	for (int i = 0; i < 3; i++)
		coord[i] = dot(p, Axis[i]) / Extent[i];

	return coord;
}

SurfaceMesh::Vector3 Box::getUniformPosition( Vector3 coord )
{
	Vector3 pos = Center;
	for (int i = 0; i < 3; i++)
		pos += coord[i] * Extent[i] * Axis[i];

	return pos;
}

