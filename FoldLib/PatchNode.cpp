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
	mPatchColor = other.mPatchColor;
}


PatchNode::~PatchNode()
{

}


void PatchNode::createPatch()
{
	int aid = mBox.minAxisId();
	mPatch = mBox.getPatch(aid, 0);
}



void PatchNode::draw()
{
	if (isCtrlPanel)
		mPatchColor = Qt::red;

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