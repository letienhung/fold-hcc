#pragma once

#include "Plane.h"
#include "Segment.h"
#include "Segment2.h"
#include <QStringList>
#include "Rectangle2.h"

namespace Geom{

class Rectangle
{
public:
    Rectangle();
    Rectangle(QVector<Vector3>& conners);
	Rectangle(Vector3& c, QVector<Vector3>& a, Vector2& e);
	Rectangle(const Rectangle &);

	// relations
	bool isCoplanarWith(Vector3 p);
	bool isCoplanarWith(Segment s);
    bool isCoplanarWith(Rectangle& other);
	bool contains(Vector3 p);
	bool contains(Segment s);
    bool contains(Rectangle& other);
	bool containsOneEdge(Rectangle& other);

	// geometry
	static int EDGE[4][2];
	double  area();
	double	radius();
	Plane   getPlane();
	int		getClosestAxisId(Vector3 v);
	int		getPerpAxisId(Vector3 v);
	QVector<Segment>  getEdgeSegments();
	QVector<Segment>  getPerpEdges(Vector3 v);
	QVector<Vector2>  get2DConners();
	QVector<Segment2> get2DEdges();
	QVector<Vector3>  getConners();
	QVector<Vector3>  getConnersReverse();

	// projection
	Rectangle2 get2DRectangle(Rectangle& rect);
	Rectangle get3DRectangle(Rectangle2 &rect2);
	Segment2 get2DSegment(Segment& s);
	Segment get3DSegment(Segment2& s);
	Vector3 getProjection(Vector3 p);
	Vector3 getProjectedVector(Vector3 v);

	// coordinates
	Vector2 getProjCoordinates(Vector3 p);
	Vector3 getPosition(const Vector2& c);
	Vector3 getVector(const Vector2& v);
	Vector2 getOpenProjCoord(Vector3 p);
	Vector3 getOpenPos(const Vector2& c);
	Vector3 getOpenVector(const Vector2& v);

	// samples
	QVector<Vector3> getEdgeSamples(int N);

	// visualization
	void drawFace(QColor color = Qt::red);
	void drawBackFace(QColor color = Qt::red);
	void drawEdges(double width = 2.0, QColor color = Qt::red);

	// modifier
	void translate(Vector3 t);
	void scale(double s);

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