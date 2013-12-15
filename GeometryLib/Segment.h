#pragma once

#include "UtilityGlobal.h"

namespace Geom{

enum {SEG_NEGATIVE, SEG_ON, SEG_POSITIVE, SEG_OFF};

class Segment
{
public:
	// constructor
    Segment();
	Segment(Vector3 p0, Vector3 p1);

	// setter
	void setFromEnds(Vector3 p0, Vector3 p1);
	void computeEndPoints();
	void computeCenterDirectionExtent();

	// end points 
	// this is a open set (P0, P1)
	// which is useful for intersection
	Vector3 P0, P1;

	// center-extent representation
	Vector3 Center, Direction;
	double	Extent;

	// coordinates
	// coord(P0) = 0, coord(P1) = 1
	double	getProjectedCoordinate(Vector3 p);
	Vector3 getPosition(double coord);
	int		whichSide(Vector3 p);

	// relation with other
	bool isCollinearWith(Vector3 p);
	bool isCollinearWith(const Segment& other);
	bool overlaps(const Segment& other);
	bool contains(Vector3 p);
	bool contains(const Segment& other);

	// intersection info that can be queried
	// point intersection: IT0
	// segment intersection: IT0, IT1
	Vector3 IT0, IT1;
};

}