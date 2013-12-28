#pragma once

#include "Plane.h"
#include "Segment.h"
#include "Segment2.h"
#include <QStringList>

namespace Geom{

class Rectangle
{
public:
    Rectangle();
    Rectangle(QVector<Vector3>& conners);
	Rectangle(Vector3& c, QVector<Vector3>& a, Vector2& e);
	Rectangle(const Rectangle &);

	void update(QVector<Vector3>& conners);
	
	// relations
	bool isCoplanarWith(Vector3 p);
	bool isCoplanarWith(Segment s);
    bool isCoplanarWith(Rectangle& other);
	bool contains(Vector3 p);
	bool contains(Segment s);
    bool contains(Rectangle& other);

	// geometry
	double  area();
	Plane   getPlane();
	Vector3 getPerpAxis(Vector3 v);
	QVector<Segment>  getEdges();
	QVector<Segment>  getPerpEdges(Vector3 v);
	QVector<Vector2>  get2DConners();
	QVector<Segment2> get2DEdges();
	QVector<Vector3>  getConners();
	QVector<Vector3>  getConnersReverse();

	// projection
	Segment2 getProjection2D(Segment s);
	Vector3 getProjection(Vector3 p);
	Vector3 getProjectedVector(Vector3 v);

	// coordinates
	Vector2 getProjCoordinates(Vector3 p);
	Vector3 getPosition(const Vector2& c);

	// visualization
	void draw(QColor color = Qt::red);
	void drawBackFace(QColor color = Qt::red);

	// to string
	QStringList toStrList();

public:
	Vector3				Center;
	QVector<Vector3>	Axis;
	Vector2				Extent;
	Vector3				Normal;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}