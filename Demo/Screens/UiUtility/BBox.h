#pragma once

//#include "FoldabilizerLibGlobal.h"
#include <QGLWidget>
#include "Line.h"

//		  7-----------6                     Y
//		 /|          /|                   f2^   /f5
//		4-+---------5 |                     |  / 
//		| |         | |                     | /
//		| |         | |             f1      |/     f0
//		| 3---------+-2            ---------+-------> X 
//		|/          |/                     /|
//		0-----------1                     / |
//								       f4/  |f3
//	                                    Z

class BBox
{
public:
	BBox(){}
	BBox(const BBox&);
	BBox(const Point& c, const Vector3& ext);
	BBox(const Point& c, const QVector<Vector3>& axis, const Vector3& ext);
	~BBox(){}

	Point Center;
	QVector<Vector3> Axis;
	Vector3 Extent;

	// Selected plane and axis
	bool isSelected;
	QVector<Point> selPlane;
	int axisID;

	Point bbmax, bbmin;

	BBox &operator =(const BBox &);

	Vector3 getCoordinates(Vector3 p);
	Vector3 getPosition(Vector3 coord);

	Vector3 getUniformCoordinates(Vector3 p);
	Vector3 getUniformPosition(Vector3 coord);

	QVector<Point> getBoxCorners();
	QVector<Geom::Line> getEdges();
	QVector<QVector<Point>> getBoxFaces();
	
	int getOrthoAxis(QVector<Point> &plane);
	
	bool IntersectRayBox(Point &start, Vec3d &startDir, Point &intPnt);
	
	void computeBBMinMax();

	// Check if intersection point is contained in the face
	bool isFaceContainPnt(Point &pnt);

	// Get idx of axis to fold after intersection
	int manipulate(Point &start, Vec3d &startDir);

	void deform(double factor); 
	void draw();
	static void DrawSquare(QVector<Point> &f, bool isOpaque, float lineWidth, const Vec4d &color);
	
};

