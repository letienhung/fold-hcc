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

FdNode::~FdNode()
{

}


void FdNode::drawCuboid()
{
	PolygonSoup ps;
	foreach(QVector<Point> f, mBox.getFacePoints()) 
		ps.addPoly(f, mColor);

	// draw faces
	ps.drawQuads(true);
	QColor c = isSelected ? Qt::yellow : Qt::white;
	ps.drawWireframes(2.0, c);
}


void FdNode::drawMesh()
{
	QuickMeshDraw::drawMeshSolid(mMesh.data());
}


void FdNode::draw()
{
	if (showMesh) drawMesh();

	// transparent
	if (showCuboids) drawCuboid();
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
	return Geom::AABB(mMesh.data());
}

void FdNode::drawWithName( int name )
{
	glPushName(name);
	drawCuboid();
	glPopName();
}

SurfaceMeshModel* FdNode::getMesh()
{
	return mMesh.data();
}