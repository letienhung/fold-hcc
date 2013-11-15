#pragma once

#include "FoldabilizerLibGlobal.h"
#include "Plane.h"

struct Box 
{
	// core data
	Point Center;
	QVector<Vector3> Axis;
	Vector3 Extent;

	// con(de)structor
	Box(){}
	Box(const Point& c, const QVector<Vector3>& axis, const Vector3& ext);
	~Box(){}

	// assignment
	Box &operator =(const Box &);

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
	static int quadFace[6][4];
	static int triFace[12][3];
	QVector<Point>				getConners();
	QVector< QVector<Point> >	getFacePoints();
	QVector<Plane>				getFacePlanes();
};


