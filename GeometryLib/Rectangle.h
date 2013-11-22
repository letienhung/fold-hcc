#pragma once

#include "Plane.h"
#include "Segment.h"
#include "Segment2.h"

namespace Geom{

class Rectangle
{
public:
    Rectangle();
    Rectangle(QVector<Vector3>& conners);
	Rectangle(Vector3& c, QVector<Vector3>& a, Vector2& e);
	
	// relations
	bool isCoplanarWith(Vector3 p);
	bool isCoplanarWith(Segment s);
    bool isCoplanarWith(const Rectangle& other);
	bool contains(Vector3 p);
	bool contains(Segment s);
    bool contains(const Rectangle& other);

	// geometry
	Plane getPlane();
	QVector<Segment> getEdges();
	QVector<Vector2> get2DConners();
	QVector<Segment2> get2DEdges();
	Segment2 getProjection2D(Segment s);

	// coordinates
	Vector2 getProjCoordinates(Vector3 p);
	Vector3 getPosition(const Vector2& c);

	// coordinates
	Vector2 getUniformCoordinates(Vector3 p);


	Vector3				Center;
	QVector<Vector3>	Axis;
	Vector2				Extent;
	Vector3				Normal;
	QVector<Vector3>	Conners;
};

}