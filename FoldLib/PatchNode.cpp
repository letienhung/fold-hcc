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
}

PatchNode::PatchNode(RodNode* rodNode, Vector3 v)
	: FdNode(rodNode->mID)
{
	mType = FdNode::PATCH;

	// create new box
	Vector3 X = rodNode->mRod.Direction;
	Vector3 Y = cross(X, v).normalized();
	Geom::Frame frame(rodNode->mBox.Center, X, Y);
	double extX = rodNode->mRod.length() / 2;
	int aid = rodNode->mBox.getAxisId(Y);
	double extY = rodNode->mBox.Extent[aid];
	mBox = Geom::Box(frame, Vector3(extX, extY, extY));

	// mesh
	mMesh = rodNode->mMesh;
	encodeMesh();

	// scaffold
	mAid= mBox.getAxisId(v);
	createScaffold(true);
}


PatchNode::PatchNode(PatchNode& other)
	:FdNode(other)
{
	mPatch = other.mPatch;
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
	mPatch.drawFace(mColor);
	mPatch.drawEdges(2.0, mColor.darker());
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
		Vector3 inwardV = (mPatch.Center - edge.Center).normalized();
		Geom::Frame frame(edge.Center + r * inwardV, mPatch.Normal, edge.Direction);
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
