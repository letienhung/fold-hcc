#include "RodNode.h"
#include "Box.h"
#include "CustomDrawObjects.h"

RodNode::RodNode(MeshPtr m, Geom::Box &b)
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

	Vector3 fc0 = mBox.getFaceCenter(fid0);
	Vector3 fc1 = mBox.getFaceCenter(fid1);

	mRod.setFromEnds(fc0, fc1);
}


void RodNode::draw()
{
	if (showScaffold)
	{
		mRod.draw(3.0, Qt::blue);
	}

	FdNode::draw();
}

void RodNode::refit( int method )
{
	FdNode::refit(method);

	// update rod
	createRod();
}
