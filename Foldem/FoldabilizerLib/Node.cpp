#include "Node.h"
#include "../CustomDrawObjects.h"

#include <QDebug>

Node::Node(Box b, QString id)
{
	mBox = b;
	mID = id;
	mColor = qRandomColor();
}

Node::~Node()
{
}


void Node::draw()
{
	drawBox();
}

QVector<Point> Node::getBoxConners()
{
	QVector<Point> pnts(8);

	// Create right-hand system
	if ( dot(cross(mBox.Axis[0], mBox.Axis[1]), mBox.Axis[2]) < 0 ) 
		mBox.Axis[2]  = -mBox.Axis[2];

	std::vector<Vec3d> Axis;
	for (int i=0;i<3;i++)
		Axis.push_back( 2 * mBox.Extent[i] * mBox.Axis[i]);

	pnts[0] = mBox.Center - 0.5*Axis[0] - 0.5*Axis[1] + 0.5*Axis[2];
	pnts[1] = pnts[0] + Axis[0];
	pnts[2] = pnts[1] - Axis[2];
	pnts[3] = pnts[2] - Axis[0];

	pnts[4] = pnts[0] + Axis[1];
	pnts[5] = pnts[1] + Axis[1];
	pnts[6] = pnts[2] + Axis[1];
	pnts[7] = pnts[3] + Axis[1];

	return pnts;
}

QVector< QVector<Point> > Node::getBoxFaces()
{
	QVector< QVector<Point> > faces(6);
	QVector<Point> pnts = getBoxConners();

	for (int i = 0; i < 6; i++)	{
		for (int j = 0; j < 4; j++)	{
			faces[i].push_back( pnts[ cubeIds[i][j] ] );
		}
	}	

	return faces;
}

void Node::drawBox()
{
	QVector< QVector<Point> > faces = getBoxFaces();

	PolygonSoup ps;
	foreach(QVector<Point> f, faces) ps.addPoly(f, mColor);
	ps.drawQuads(true);
}

