#pragma once

#include "Segment.h"

namespace Goem{

class Plane;

class Rectangle
{
public:
    Rectangle();
    Rectangle(QVector<Vector3>& conners);
	
	// relations
	bool isCoplanarWith(Vector3 p);
	bool isCoplanarWith(Segment s);
	bool contains(Vector3 p);
	bool contains(Segment s);
	bool contains(const Rectangle& other);

	Plane getPlane();

	// coordinates
	Vector2 getUniformCoordinates(Vector3 p);


	Vector3				Center;
	QVector<Vector3>	Axis;
	Vector2				Extent;
	Vector3				Normal;
	QVector<Vector3>	Conners;
};

}