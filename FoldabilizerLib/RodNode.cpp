#include "RodNode.h"
#include "Box.h"
#include "CustomDrawObjects.h"

RodNode::RodNode(SurfaceMeshModel* m, Geom::Box &b)
	: FdNode(m, b)
{
	mType = FdNode::ROD;
	createRod();
}

RodNode::~RodNode()
{

}


void RodNode::createRod()
{
	int aid = mBox.maxAxisId();
	int fid0 = mBox.getFaceId(aid, true);
	int fid1 = mBox.getFaceId(aid, false);

	rod.P0 = mBox.getFaceCenter(fid0);
	rod.P1 = mBox.getFaceCenter(fid1);
}


void RodNode::draw()
{
	if (showScaffold)
	{
		LineSegments ls(3.0f);
		ls.addLine(rod.P0, rod.P1);
		ls.draw();
	}

	FdNode::draw();
}

void RodNode::refit( int method )
{
	FdNode::refit(method);

	// update rod
	createRod();
}
