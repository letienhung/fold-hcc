#include "RodNode.h"
#include "Box.h"
#include "CustomDrawObjects.h"

RodNode::RodNode(QString id, Geom::Box &b, MeshPtr m)
	: ScaffNode(id, b, m)
{
	mType = ScaffNode::ROD;
	createScaffold(false);
}

RodNode::RodNode(RodNode& other)
	:ScaffNode(other)
{
	mRod = other.mRod;
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
	mRod.draw(10.0, mColor.darker(), false);
	mRod.draw(8.0, mColor, false);
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

QVector<Vector3> RodNode::sampleBoundabyOfScaffold(int n)
{
	return mRod.getUniformSamples(n);
}
