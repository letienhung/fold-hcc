#include "Node.h"
#include "CustomDrawObjects.h"

#include <QDebug>

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
}

Frame Node::getFrame()                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
{
	return mBox.getFrame();
}

void Node::setFrame( Frame f )
{
	mBox.setFrame(f);
}

void Node::translate( Vector3 t )
{
	mBox.Center += t;
}



void Node::fix()
{
	for (int i = 0 ; i < 3; i++)
	{
		mBox.Extent[i] = scaleFactor[i] * originalExtent[i];
	}
	
}

double Node::getVolume()
{
	Vector3 e = 2 * mBox.Extent;
	return e[0] * e[1] * e[2];
}

