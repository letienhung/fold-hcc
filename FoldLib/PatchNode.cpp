#include "PatchNode.h"
#include "Segment.h"
#include "Box.h"
#include "CustomDrawObjects.h"

PatchNode::PatchNode(MeshPtr m, Geom::Box &b)
	: FdNode(m, b)
{
	mType = FdNode::PATCH;
	createPatch();

	mPatchColor = mColor.lighter();
	mPatchColor.setAlpha(255);
}

PatchNode::PatchNode(PatchNode& other)
	:FdNode(other)
{
	mPatch = other.mPatch;
}


PatchNode::~PatchNode()
{

}


void PatchNode::createPatch()
{
	int aid = mBox.minAxisId();
	QVector<Geom::Segment> edges = mBox.getEdgeSegmentsAlongAxis(aid);

	QVector<Vector3> conners;
	for (int i = 0; i < 4; i++)	conners.push_back(edges[i].Center);
	mPatch = Geom::Rectangle(conners);
}



void PatchNode::draw()
{
	if (showScaffold)
	{
		mPatch.draw(mPatchColor);
		mPatch.drawBackFace(mPatchColor);
	}

	FdNode::draw();
}

void PatchNode::refit(int method)
{
	FdNode::refit(method);

	// update patch
	createPatch();
}

bool PatchNode::isPerpTo( Vector3 v, double dotThreshold )
{
	double dotProd = dot(mPatch.Normal, v);
	return (fabs(dotProd) > 1 - dotThreshold);
}

Structure::Node* PatchNode::clone()
{
	return new PatchNode(*this);
}
