#include "Node.h"
#include "CustomDrawObjects.h"

#include <QDebug>

using namespace Geom;

Node::Node(Box b, QString id)
{
	mBox = b; 
	mID = id;
	mColor = qRandomColor(); mColor.setAlphaF(0.5);

	isFixed = false;

	originalExtent = mBox.Extent;
	scaleFactor = Vector3(1,1,1);

	isHighlight = false;
	isHot = false;
	isFocused = false;
}

Node::~Node()
{
}

void Node::draw()
{
	PolygonSoup ps;
	foreach(QVector<Point> f, mBox.getFacePoints()) ps.addPoly(f, mColor);

	// draw faces
	ps.drawQuads(true);

	// highlight wireframes
	if (isFixed) 
		ps.drawWireframes(2.0, Qt::white);
	else if(isHot) 
		ps.drawWireframes(2.0, Qt::red);
	else if(isHighlight) 
		ps.drawWireframes(2.0, Qt::yellow);
	else if(isFocused)
		ps.drawWireframes(2.0, Qt::magenta);	
}


void Node::drawDebug()
{
	// debug points
	PointSoup dps;
	foreach(Vector3 v, this->debug_points) dps.addPoint(v);
	dps.draw();
}


void Node::translate( Vector3 t )
{
	mBox.Center += t;
}

void Node::rotate( qglviewer::Quaternion &q )
{
	qglviewer::Vec axis0, axis1, axis2;

	axis0 = q.rotate(qglviewer::Vec(mBox.Axis[0].x(), mBox.Axis[0].y(), mBox.Axis[0].z()));
	axis1 = q.rotate(qglviewer::Vec(mBox.Axis[1].x(), mBox.Axis[1].y(), mBox.Axis[1].z()));
	axis2 = q.rotate(qglviewer::Vec(mBox.Axis[2].x(), mBox.Axis[2].y(), mBox.Axis[2].z()));

	mBox.Axis[0] = Vector3(axis0[0], axis0[1], axis0[2]);
	mBox.Axis[1] = Vector3(axis1[0], axis1[1], axis1[2]);
	mBox.Axis[2] = Vector3(axis2[0], axis2[1], axis2[2]);
}

void Node::fix()
{
	for (int i = 0 ; i < 3; i++)
	{
		mBox.Extent[i] = scaleFactor[i] * originalExtent[i];
	}
	
}

