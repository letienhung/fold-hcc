#include "PatchNode.h"
#include "Segment.h"
#include "Box.h"
#include "CustomDrawObjects.h"

PatchNode::PatchNode(MeshPtr m, Geom::Box &b)
	: FdNode(m, b)
{
	mType = FdNode::PATCH;
	createScaffold();

	mPatchColor = mColor.lighter();
	mPatchColor.setAlpha(255);
}

PatchNode::PatchNode(PatchNode& other)
	:FdNode(other)
{
	mPatch = other.mPatch;
	mPatchColor = other.mPatchColor;
}


void PatchNode::createScaffold()
{
	int aid = mBox.minAxisId();
	mPatch = mBox.getPatch(aid, 0);
}



void PatchNode::draw()
{
	if (properties.contains("isCtrlPanel"))
		mPatchColor = Qt::red;

	if (showScaffold)
	{
		mPatch.draw(mPatchColor);
		mPatch.drawBackFace(mPatchColor);
	}

	FdNode::draw();
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

Geom::Plane PatchNode::getSurfacePlane( bool positive)
{
	int aid = mBox.getAxisId(mPatch.Normal);
	double extent = mBox.getExtent(aid);
	Geom::Plane plane = mPatch.getPlane();

	if (positive) plane.translate(extent * mPatch.Normal);
	else {
		plane.translate(-extent * mPatch.Normal);
		plane = plane.opposite();
	}

	return plane;
}
