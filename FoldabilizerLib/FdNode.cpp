#include "FdNode.h"
#include "CustomDrawObjects.h"

FdNode::FdNode( SurfaceMeshModel *m, Geom::Box &b )
	: Node(m->name), mesh(m), mBox(b)
{
	mColor = qRandomColor(); 
	mColor.setAlphaF(0.5);

	mType = NONE;
}

void FdNode::draw()
{
	PolygonSoup ps;
	foreach(QVector<Point> f, mBox.getFacePoints()) 
		ps.addPoly(f, mColor);

	// draw faces
	ps.drawQuads(true);
	ps.drawWireframes(2.0, Qt::yellow);
}
