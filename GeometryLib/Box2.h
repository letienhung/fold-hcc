#pragma once

#include "Segment.h"

class Box2
{
public:
    Box2();
    Box2(QVector<Vector3>& conners);
	
	// relations
	bool isCoplanar(Vector3 p);
	bool isCoplanar(Segment s);
	bool contains(Vector3 p);
	bool contains(Segment s);

	// coordinates
	Vector2 getUniformCoordinates(Vector3 p);

private:
	Vector3				Center;
	QVector<Vector3>	Axis;
	Vector2				Extent;
	Vector3				Normal;
	QVector<Vector3>	Conners;
};
