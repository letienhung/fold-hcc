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
    bool isCoplanarWith(Rectangle& other);
	bool contains(Vector3 p);
	bool contains(Segment s);
    bool contains(Rectangle& other);

	// geometry
	double area();
	Plane getPlane();
	QVector<Segment> getEdges();
	QVector<Vector2> get2DConners();
	QVector<Segment2> get2DEdges();
	Segment2 getProjection2D(Segment s);
	QVector<Vector3> getConners();
	QVector<Vector3> getConnersReverse();

	// coordinates
	Vector2 getProjCoordinates(Vector3 p);
	Vector3 getPosition(const Vector2& c);
	Vector2 getCoordinates(Vector3 p);

	// visualization
	void draw(QColor color = Qt::red);
	void drawBackFace(QColor color = Qt::red);

public:
	Vector3				Center;
	QVector<Vector3>	Axis;
	Vector2				Extent;
	Vector3				Normal;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}