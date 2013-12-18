#include "RodNode.h"
#include "Box.h"
#include "CustomDrawObjects.h"

RodNode::RodNode(MeshPtr m, Geom::Box &b)
	: FdNode(m, b)
{
	mType = FdNode::ROD;
	createRod();

	mRodColor = mColor.lighter();
	mRodColor.setAlpha(255);
}

RodNode::RodNode(RodNode& other)
	:FdNode(other)
{
	mRod = other.mRod;
}

RodNode::~RodNode()
{

}


void RodNode::createRod()
{
	int aid = mBox.maxAxisId();
	mRod = mBox.getSkeleton(aid);
}


void RodNode::draw()
{
	if (showScaffold)
	{
		mRod.draw(3.0, mRodColor);
	}

	FdNode::draw();
}

void RodNode::refit( int method )
{
	FdNode::refit(method);

	// update rod
	createRod();
}

bool RodNode::isPerpTo( Vector3 v, double dotThreshold )
{
	double dotProd = dot(mRod.Direction, v);
	return (fabs(dotProd) < dotThreshold);
}

Structure::Node* RodNode::clone()
{
	return new RodNode(*this);
}
