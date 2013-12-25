#pragma once

#include "Segment.h"
#include "Rectangle.h"

namespace Geom{

class SectorCylinder
{
public:
    SectorCylinder(Segment a, Segment r1, Vector3 v2);

	Geom::Segment getAxisSegment();

	bool intersects(Segment& seg);
	bool intersects(Rectangle& rect);

public:
	// (Origin, V1, V2) forms the bottom sector
	// Axis is up
	Vector3 Origin;
	Vector3 V1, V2, Axis;
	double Radius, Height;
};

}