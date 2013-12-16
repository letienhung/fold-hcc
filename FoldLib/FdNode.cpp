#include "FdNode.h"
#include "CustomDrawObjects.h"
#include "Numeric.h"
#include "FdUtility.h"
#include "AABB.h"
#include "MinOBB.h"
#include "QuickMeshDraw.h"

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
}


FdNode::~FdNode()
{

}

void FdNode::draw()
{
	if (showMesh)
		QuickMeshDraw::drawMeshSolid(mMesh.data());

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
	meshCoords.clear();
	foreach(Vector3 p, getMeshVertices(mMesh.data()))
		meshCoords.push_back(origBox.getCoordinates(p));
}

void FdNode::deformMesh()
{
	Surface_mesh::Vertex_property<Point> points = mMesh->vertex_property<Point>("v:point");

	for(int i = 0; i < (int)mMesh->n_vertices(); i++)
	{
		Surface_mesh::Vertex vit(i);
		points[vit] = mBox.getPosition(meshCoords[i]);
	}
}

void FdNode::updateBox()
{

}

void FdNode::writeToXml( XmlWriter& xw )
{
	xw.writeOpenTag("node");
	{
		xw.writeTaggedString("type", QString::number(mType));
		xw.writeTaggedString("ID", this->id);

		// box
		writeBoxToXml(xw, mBox);

		// scaffold
		this->writeScaffoldToXml(xw);
	}
	xw.writeCloseTag("node");
}

void FdNode::refit( int method )
{
	switch(method)
	{
	case 0: // OBB
		{
			Geom::MinOBB obb(mMesh.data());
			mBox = obb.mMinBox;
		}
		break;
	case 1: // AABB
		{
			Geom::AABB aabb(mMesh.data());
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
	return false;
}
