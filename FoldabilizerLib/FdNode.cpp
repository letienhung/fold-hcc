#include "FdNode.h"
#include "CustomDrawObjects.h"

FdNode::FdNode( SurfaceMeshModel *m, Geom::Box &b )
	: Node(m->name)
{
	this->mesh = m;
	this->ctrlBox = b;
	mColor = qRandomColor(); 
	mColor.setAlphaF(0.5);

	CH = new Geom::ConvexHull(mesh);
}

void FdNode::draw()
{
	//CH->draw();
	//return;

	PolygonSoup ps;
	foreach(QVector<Point> f, ctrlBox.getFacePoints()) 
		ps.addPoly(f, mColor);

	// draw faces
	ps.drawQuads(true);
	ps.drawWireframes(2.0, Qt::yellow);

}
