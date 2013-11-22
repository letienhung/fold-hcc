#pragma once

#include "Box.h"
#include "Link.h"

class Node
{
public:
	QString		mID;
	Geom::Box	mBox;
	QColor		mColor;
	
	bool		isHot;
	bool		isFixed;		// tag used for propagation
	Vector3		originalExtent;
	Vector3		scaleFactor;	// scale factor of box

    Node(Geom::Box b, QString id);
    ~Node();

public:
	// Geometry
	double		getVolume();
	Geom::Frame	getFrame();
	void		setFrame(Geom::Frame f);

	// Transformation
	void translate(Vector3 t);
	void fix();

	// Visualize
	bool isHighlight;
	void draw();

	// debug
	QVector<Vector3> debug_points;
};                                 
