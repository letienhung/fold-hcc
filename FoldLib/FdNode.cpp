#include "FdNode.h"
#include "CustomDrawObjects.h"
#include "Numeric.h"
#include "AABB.h"
#include "MinOBB.h"
#include "QuickMeshDraw.h"
#include "MeshHelper.h"
#include "MeshBoolean.h"

FdNode::FdNode( MeshPtr m, Geom::Box &b )
	: Node(m->name)
{
	mMesh = m;

	origBox = b;
	mBox = b;

	mColor = qRandomColor(); 
	mColor.setAlphaF(0.5);
	mType = NONE;

	showCuboids = true;
	showScaffold = true;
	showMesh = false;

	isCtrlPanel = false;
}

FdNode::FdNode(FdNode& other)
	:Node(other)
{
	mMesh = other.mMesh;
	origBox = other.origBox;
	mBox = other.mBox;
	mColor = other.mColor;
	mType = other.mType;

	showCuboids = true;
	showScaffold = true;
	showMesh = false;

	isCtrlPanel = false;
}


FdNode::~FdNode()
{

}

void FdNode::draw()
{
	if (showMesh)
	{
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
		else			
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

		// scaffold
		writeScaffold(xw);
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
	}

	// encode mesh
	origBox = mBox;
	encodeMesh();
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

Structure::Node* FdNode::clone()
{
	return new FdNode(*this);
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
