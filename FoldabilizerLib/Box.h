#pragma once

#include "FoldabilizerLibGlobal.h"
#include "Frame.h"
#include "Plane.h"
#include "Line.h"
#include "Segment.h"
#include "Rectangle.h"

struct Box 
{
	// core data
	Point Center;
	QVector<Vector3> Axis;
	Vector3 Extent;

	// con(de)structor
	Box(){}
	~Box(){}
	Box(const Point& c, const QVector<Vector3>& axis, const Vector3& ext);

	// assignment
	Box &operator =(const Box &);

	// frame
	Frame	getFrame();
	void	setFrame(Frame f);

	// coordinates
	Vector3 getCoordinates(Vector3 p);
	Vector3 getPosition(Vector3 coord);

	Vector3 getUniformCoordinates(Vector3 p);
	Vector3 getUniformPosition(Vector3 coord);

	// transform
	void translate(Vector3 t);
	void uniformScale(double s);
	void scale(Vector3 s);

	// geometry
	static int EDGE[12][2];
	static int QUAD_FACE[6][4];
	static int TRI_FACE[12][3];
	QVector<Point>				getConnerPoints();
	QVector<Line>				getEdgeLines();
	QVector<Segment>			getEdgeSegments();
	QVector< QVector<Point> >	getFacePoints();
	QVector<Plane>				getFacePlanes();
	QVector<Rectangle>			getFaceRectangles();

	// relation with other objects
	bool onBox(Line line);
};


