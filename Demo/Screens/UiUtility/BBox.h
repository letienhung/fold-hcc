#pragma once

//#include "FoldabilizerLibGlobal.h"
#include <QGLWidget>
#include "Line.h"
#include "Rectangle.h"
#include "CustomDrawObjects.h"

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
	QVector<Geom::Rectangle> mFaces;
	QVector<Vector3> Axis;
	Vector3 Extent;

	// Selected plane and axis
	bool isSelected;
	int selPlaneID;
	int axisID;

	Point bbmax, bbmin;

	BBox &operator =(const BBox &);

	Vector3 getCoordinates(Vector3 p);
	Vector3 getPosition(Vector3 coord);

	Vector3 getUniformCoordinates(Vector3 p);
	Vector3 getUniformPosition(Vector3 coord);

	QVector<Point> getBoxCorners();
	QVector<Geom::Line> getEdges();
	void getBoxFaces();
	
	void getOrthoAxis(Geom::Rectangle &plane);
	
	bool IntersectRayBox(Point &start, Vec3d &startDir, Point &intPnt);
	
	void computeBBMinMax();

	bool contain(Point &p);

	// Check if intersection point is contained in the face
	bool isFaceContainPnt(Point &pnt);

	// Get selection information by intersection detection
	void selectFace(Point &start, Vec3d &startDir);

	Geom::Rectangle getSelectedFace();

	// Return id of parallel face of f.
	int getParallelFace(Geom::Rectangle &f);
	void deform(double f); 
	void draw();
	static void DrawSquare(Geom::Rectangle &f, bool isOpaque, float lineWidth, const Vec4d &color);
	
};

