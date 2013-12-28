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

	Geom::Segment getAxisSegment();
	Vector3 getCoordinates(Vector3 p);
	bool contains(Vector3 p);

	bool intersects(Segment& seg);
	bool intersects(Rectangle& rect);

	QStringList toStrList();

public:
	// (Origin, V1, V2) forms the bottom sector
	// Axis is up
	Vector3 Origin;
	Vector3 V1, V2, Axis;
	double Height, Radius, Phi;
};

}