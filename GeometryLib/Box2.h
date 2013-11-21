#pragma once

#include "Segment.h"

class Plane;

class Box2
{
public:
    Box2();
    Box2(QVector<Vector3>& conners);
	
	// relations
	bool isCoplanarWith(Vector3 p);
	bool isCoplanarWith(Segment s);
	bool contains(Vector3 p, bool isOpen = false);
	bool contains(Segment s, bool isOpen = false);
	Plane getPlane();

	// coordinates
	Vector2 getUniformCoordinates(Vector3 p);


	Vector3				Center;
	QVector<Vector3>	Axis;
	Vector2				Extent;
	Vector3				Normal;
	QVector<Vector3>	Conners;
};
