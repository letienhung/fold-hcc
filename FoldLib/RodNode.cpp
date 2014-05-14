#include "RodNode.h"
#include "Box.h"
#include "CustomDrawObjects.h"

RodNode::RodNode(QString id, Geom::Box &b, MeshPtr m)
	: FdNode(id, b, m)
{
	mType = FdNode::ROD;
	createScaffold(false);

	mRodColor = mColor.lighter();
	mRodColor.setAlpha(255);
}

RodNode::RodNode(RodNode& other)
	:FdNode(other)
{
	mRod = other.mRod;
	mRodColor = other.mRodColor;
}

RodNode::~RodNode()
{

}


void RodNode::createScaffold(bool useAid)
{
	if (!useAid)
		mAid = mBox.maxAxisId();

	mRod = mBox.getSkeleton(mAid);
}


void RodNode::drawScaffold()
{
	mRod.draw(3.0, mRodColor);
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
