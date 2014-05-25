#include "PatchNode.h"
#include "Segment.h"
#include "Box.h"
#include "CustomDrawObjects.h"
#include "RodNode.h"

PatchNode::PatchNode(QString id, Geom::Box &b, MeshPtr m)
	: FdNode(id, b, m)
{
	mType = FdNode::PATCH;
	createScaffold(false);

	mPatchColor = mColor;
	mPatchColor.setAlphaF(1.0);
}

PatchNode::PatchNode(QString id, Geom::Box &b, Vector3 v, MeshPtr m)
	: FdNode(id, b, m)
{
	mType = FdNode::PATCH;

	mAid= mBox.getAxisId(v);
	createScaffold(true);

	mPatchColor = mColor.lighter();
	mPatchColor.setAlpha(255);
}

PatchNode::PatchNode(PatchNode& other)
	:FdNode(other)
{
	mPatch = other.mPatch;
	mPatchColor = other.mPatchColor;
}


void PatchNode::createScaffold(bool useAid)
{
	// update axis index
	if (!useAid)
		mAid = mBox.minAxisId();

	mPatch = mBox.getPatch(mAid, 0);
}

void PatchNode::drawScaffold()
{
	if (hasTag(MASTER_TAG))
		mPatchColor = Qt::red;

	mPatch.drawFace(mPatchColor);
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

QVector<RodNode*> PatchNode::getEdgeRodNodes()
{
	QVector<RodNode*> edgeRods;
	double r = 0.5 * getThickness();
	int i = 0;
	Geom::Box mBox_copy = mBox;
	foreach (Geom::Segment edge, mPatch.getEdgeSegments())
	{
		// box
		Geom::Frame frame(edge.Center, mPatch.Normal, edge.Direction);
		Vector3 extent(r, edge.Extent, r);
		Geom::Box box(frame, extent);

		// deform mesh
		mBox = box;
		deformMesh();

		// create rod node
		QString id = mID + "er" + QString::number(i++);
		edgeRods << new RodNode(id, box, mMesh);

		// restore mesh
		mBox = mBox_copy;
		deformMesh();
	}

	return edgeRods;
}

double PatchNode::getThickness()
{
	int aid = mBox.getAxisId(mPatch.Normal);
	return 2 * mBox.getExtent(aid);
}

void PatchNode::resize( Geom::Rectangle2& newPatch )
{
	Geom::Rectangle newPatch3 = mPatch.get3DRectangle(newPatch);

	// shift center
	mBox.translate(newPatch3.Center - mPatch.Center);

	// adjust extent
	int aid_x = mBox.getAxisId(newPatch3.Axis[0]);
	mBox.Extent[aid_x] = newPatch3.Extent[0];

	int aid_y = mBox.getAxisId(newPatch3.Axis[1]);
	mBox.Extent[aid_y] = newPatch3.Extent[1];

	// update scaffold
	createScaffold(true);
}

void PatchNode::setThickness( double thk )
{
	int aid = mBox.getAxisId(mPatch.Normal);
	mBox.Extent[aid] = 0.5 * thk;
}
