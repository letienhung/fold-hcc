#pragma once

#include "Segment.h"
#include "Rectangle.h"
#include <QStringList>

namespace Geom{

class SectorCylinder
{
public:
	SectorCylinder();
    SectorCylinder(Segment a, Segment r1, Vector3 v2);
	SectorCylinder(Vector3 o, Vector3 x, Vector3 y, Vector3 z, double h, double r);

	Geom::Segment getAxisSegment();
	Vector3 getCoordinates(Vector3 p);
	Vector3 getPosition(Vector3 coord);
	bool contains(Vector3 p);

	bool intersects(Segment& seg);
	bool intersects(Rectangle& rect);

	void translate(Vector3 t);

	QStringList toStrList();

	// shrink the epsilon from the boundary
	void shrinkEpsilon();

	// conner
	QVector<Vector3> getConners();

	// visual
	void draw();

public:
	// (Origin, V1, V2) forms the bottom sector
	// Axis is up
	Vector3 Origin;
	Vector3 V1, V2, Axis;
	double Height, Radius, Phi;
};

}