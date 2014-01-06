#include "FdNode.h"
#include "CustomDrawObjects.h"
#include "Numeric.h"
#include "AABB.h"
#include "MinOBB.h"
#include "QuickMeshDraw.h"
#include "MeshHelper.h"
#include "MeshBoolean.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "PcaOBB.h"

FdNode::FdNode( MeshPtr m, Geom::Box &b )
	: Node(m->name)
{
	mMesh = m;

	origBox = b;
	mBox = b;
	encodeMesh();

	mColor = qRandomColor(); 
	mColor.setAlphaF(0.5);
	mType = NONE;

	showCuboids = true;
	showScaffold = true;
	showMesh = true;
}

FdNode::FdNode(FdNode& other)
	:Node(other)
{
	mMesh = other.mMesh;
	origBox = other.origBox;
	mBox = other.mBox;
	meshCoords = other.meshCoords;

	mColor = other.mColor;
	mType = other.mType;

	showCuboids = true;
	showScaffold = true; 
	showMesh = false;
}


FdNode::~FdNode()
{

}

void FdNode::draw()
{
	if (showMesh)
	{
		deformMesh();
		QuickMeshDraw::drawMeshSolid(mMesh.data());
		QuickMeshDraw::drawMeshWireFrame(mMesh.data());
	}

	if (showCuboids)
	{
		// faces
		mBox.draw(mColor);

		// wireframes
		if(isSelected)	
			mBox.drawWireframe(4.0, Qt::yellow);
		else if (!properties.contains("isCtrlPanel"))
			mBox.drawWireframe();
	}
}

void FdNode::encodeMesh()
{
	meshCoords = MeshHelper::encodeMeshInBox(mMesh.data(), origBox);
}

void FdNode::deformMesh()
{
	MeshHelper::decodeMeshInBox(mMesh.data(), mBox, meshCoords);
}

void FdNode::write( XmlWriter& xw )
{
	xw.writeOpenTag("node");
	{
		xw.writeTaggedString("type", QString::number(mType));
		xw.writeTaggedString("ID", this->mID);

		// box
		mBox.write(xw);
	}
	xw.writeCloseTag("node");
}

void FdNode::refit( int method )
{
	QVector<Vector3> points = MeshHelper::getMeshVertices(mMesh.data());
	switch(method)
	{
	case 0: // OBB
		{
			Geom::MinOBB obb(points, true);
			mBox = obb.mMinBox;
		}
		break;
	case 1: // AABB
		{
			Geom::AABB aabb(points);
			mBox = aabb.box();
		}
		break;
	case 2:
		{
			Geom::PcaOBB pca_obb(points);
			mBox = pca_obb.minBox;
		}
		break;
	}

	// encode mesh
	origBox = mBox;

	encodeMesh();
	createScaffold();
}

Geom::AABB FdNode::computeAABB()
{
	return Geom::AABB(mBox.getConnerPoints());
}

void FdNode::drawWithName( int name )
{
	glPushName(name);
	mBox.draw();
	glPopName();
}

bool FdNode::isPerpTo( Vector3 v, double dotThreshold )
{
	Q_UNUSED(v);
	Q_UNUSED(dotThreshold);
	return false;
}

SurfaceMesh::Vector3 FdNode::center()
{
	return mBox.Center;
}

// clone partial node on the positive side of the chopper plane
FdNode* FdNode::cloneChopped( Geom::Plane chopper )
{
	// cut point along skeleton
	int aid = mBox.getClosestAxisId(chopper.Normal);
	Geom::Segment sklt = mBox.getSkeleton(aid);

	// skip if chopper plane doesn't intersect with the node
	if (!chopper.intersects(sklt)) return NULL;

	Vector3 cutPoint = chopper.getIntersection(sklt);
	Vector3 endPoint = (chopper.signedDistanceTo(sklt.P0) > 0) ?
						sklt.P0 : sklt.P1;
	
	// skip if chopped piece is too small
	double chopLength = (cutPoint - endPoint).norm();
	if (chopLength / sklt.length() < 0.1) return NULL;

	// chop box
	Geom::Box box = mBox;
	box.Center = (cutPoint + endPoint) * 0.5;
	box.Extent[aid] = (cutPoint - endPoint).norm() * 0.5;
	
	// chop mesh
	// Geom::Box chopBox = box;
	// chopBox.Extent *= 1.001;
	// SurfaceMeshModel* mesh = MeshBoolean::cork(mMesh.data(), chopBox, MeshBoolean::ISCT);

	// create new nodes
	FdNode *choppedNode;
	if (mType == FdNode::ROD)
		choppedNode = new RodNode(mMesh, box);
	else
		choppedNode = new PatchNode(mMesh, box);
	choppedNode->meshCoords = meshCoords;

	//qDebug() << mID << "(" << mBox.volume() << ") => " << choppedNode->mID << "(" << choppedNode->mBox.volume() << ")";

	return choppedNode;
}

void FdNode::setStringId( QString id )
{
	mID = id;
	mMesh->name = id;
}
