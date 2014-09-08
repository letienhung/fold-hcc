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
	mRod.draw(3.0, mRodColor, false);
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

void RodNode::setThickness( double thk )
{
	int aid = mBox.getAxisId(mRod.Direction);
	mBox.Extent[(aid+1)%3] = thk / 2;
	mBox.Extent[(aid+2)%3] = thk / 2;
}

QVector<Vector3> RodNode::sampleBoundabyOfScaffold(int n)
{
	return mRod.getUniformSamples(n);
}
