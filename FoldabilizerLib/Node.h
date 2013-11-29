#pragma once

#include "Box.h"
#include "Link.h"

#include "qglviewer/quaternion.h"

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

//Define box edge type
typedef QPair<Point, Point> EdgeType;
//Define box face type
typedef QVector<Point> FaceType;

const double PI = 3.14159265;

class Node
{
public:
	QString		mID;
	Geom::Box	mBox;
	QColor		mColor;
	
	bool		isHot;
	bool		isFixed;		// tag used for propagation
	bool        isFocused;      // tag used for editing


	QVector<Node*> collisionList;

    Node(Geom::Box b, QString id);
    ~Node();

public:
	// Transformation
	void translate(Vector3 t);
	void rotate(qglviewer::Quaternion &q);

	// Scale box wrt the original box
	Vector3	originalExtent;
	Vector3 scaleFactor;
	void	scale(Vector3 s);
	void	scale(double s);
	void	scale(int axisId, double s);

	// Visualize
	bool isHighlight;
	void draw();
	void drawDebug();

	// debug
	QVector<Vector3> debug_points;
};                                 
