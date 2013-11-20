#pragma once

#include "UtilityGlobal.h"

enum {SEG_NEGATIVE, SEG_ON, SEG_POSITIVE};

class Segment
{
public:
	// end points 
	// this is a open set (P0, P1)
	// which is useful for intersection
	Vector3 P0, P1;

	// center-extent representation
	Vector3 Center, Direction;
	double	Extent;

	// constructor
    Segment();
	Segment(Vector3 p0, Vector3 p1);

	// coordinates
	// coord(P0) = 0, coord(P1) = 1
	double	getProjectedCoordinate(Vector3 p);
	Vector3 getPosition(double coord);
	int		whichSide(Vector3 p);

	// relation with other
	bool isCollinearWith(const Segment& other);
	bool overlaps(const Segment& other);

	// intersection info that can be queried
	// point intersection: IT0
	// segment intersection: IT0, IT1
	Vector3 IT0, IT1;
};

