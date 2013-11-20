#pragma once

#include "FoldabilizerLibGlobal.h"

#include "Box.h"
#include "Link.h"

class Node
{
public:
	QString		mID;
	Box			mBox;
	QColor		mColor;
	
	bool		isHot;
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

	// Transformation
	void translate(Vector3 t);
	void fix();

	// Visualize
	bool isHighlight;
	void draw();
};                                 