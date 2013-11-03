#include "Node.h"
#include "../CustomDrawObjects.h"

#include <QDebug>

Node::Node(Box b, QString id)
{
	mBox = b;
	mID = id;
}

Node::~Node()
{
    int mSize = linkList.size();
    if(mSize){
		for(int i = 0; i < mSize; i++)
            if(linkList[i])
                delete linkList[i];
        linkList.clear();
	}
}

QVector<Node *> Node::getAdjnodes()
{
	QVector<Node *> adjNodes;
	//TODO
	return adjNodes;
}

void Node::draw()
{
	drawBox(mBox);
}

QVector<Point> Node::getBoxConners( Box &box )
{
	QVector<Point> pnts(8);

	// Create right-hand system
	if ( dot(cross(box.Axis[0], box.Axis[1]), box.Axis[2]) < 0 ) 
		box.Axis[2]  = -box.Axis[2];

	std::vector<Vec3d> Axis;
	for (int i=0;i<3;i++)
		Axis.push_back( 2 * box.Extent[i] * box.Axis[i]);

	pnts[0] = box.Center - 0.5*Axis[0] - 0.5*Axis[1] + 0.5*Axis[2];
	pnts[1] = pnts[0] + Axis[0];
	pnts[2] = pnts[1] - Axis[2];
	pnts[3] = pnts[2] - Axis[0];

	pnts[4] = pnts[0] + Axis[1];
	pnts[5] = pnts[1] + Axis[1];
	pnts[6] = pnts[2] + Axis[1];
	pnts[7] = pnts[3] + Axis[1];

	return pnts;
}

QVector< QVector<Point> > Node::getBoxFaces( Box &box )
{
	QVector< QVector<Point> > faces(6);
	QVector<Point> pnts = getBoxConners(box);

	for (int i = 0; i < 6; i++)	{
		for (int j = 0; j < 4; j++)	{
			faces[i].push_back( pnts[ cubeIds[i][j] ] );
		}
	}	

	return faces;
}

void Node::drawBox(Box &box)
{
	QVector< QVector<Point> > faces = getBoxFaces(box);

	PolygonSoup ps;
	foreach(QVector<Point> f, faces) ps.addPoly(f, Qt::blue);
	//ps.drawQuads(true);
	ps.draw();
}
