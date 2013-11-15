#pragma once

#include "Box.h"
#include "FoldabilizerLibGlobal.h"

#include "Link.h"

//class Link;

class Node
{
public:
	QString		mID;
	Box			mBox;
	QColor		mColor;

	bool		isFixed;		// tag used for propagation
	Vector3		originalExtent;
	Vector3		scaleFactor;	// scale factor of box

    Node(Box b, QString id);
    ~Node();

public:
	// Geometry
	double	getVolume();
	Frame	getFrame();
	void	setFrame(Frame f);
	QVector<Point>				getBoxConners();
	QVector< QVector<Point> >	getBoxFaces();

	// Hinge property
	Vec3d dihedralDirection(Vec3d hinge_pos, Vec3d hinge_axis); 

	// Transformation
	void translate(Vector3 t);
	void fix();

	// Visualize
	bool isHighlight;
	void draw();
};

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


// Vertex ID of face corners
static uint quadFace[6][4] = 
{
	1, 2, 6, 5,
	0, 4, 7, 3,
	4, 5, 6, 7,
	0, 3, 2, 1,
	0, 1, 5, 4,
	2, 3, 7, 6
};

static uint triFace[12][3] = 
{
	1, 2, 6,
	6, 5, 1,
	0, 4, 7,
	7, 3, 0,
	4, 5, 6,
	6, 7, 4,
	0, 3, 2,
	2, 1, 0,
	0, 1, 5,
	5, 4, 0,
	2, 3, 7,
	7, 6, 2
};