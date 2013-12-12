#include "FdNode.h"
#include "CustomDrawObjects.h"
#include "Numeric.h"
#include "FdUtility.h"
#include "AABB.h"
#include "MinOBB.h"

FdNode::FdNode( MeshPtr m, Geom::Box &b )
	: Node(m->name), mMesh(m)
{
	origBox = b;
	mBox = b;

	mColor = qRandomColor(); 
	mColor.setAlphaF(0.5);
	mType = NONE;

	showCuboids = true;
	showScaffold = true;
}

FdNode::~FdNode()
{

}

void FdNode::draw()
{
	if (showCuboids)
	{
		PolygonSoup ps;
		foreach(QVector<Point> f, mBox.getFacePoints()) 
			ps.addPoly(f, mColor);

		// draw faces
		ps.drawQuads(true);
		QColor c = isSelected ? Qt::yellow : Qt::white;
		ps.drawWireframes(2.0, c);
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

	for(int i = 0; i < mMesh->n_vertices(); i++)
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
	case 0: // AABB
		{
			Geom::AABB aabb(mMesh.data());
			mBox = aabb.box();
		}
		break;
	case 1: // OBB
		{
			Geom::MinOBB obb(mMesh.data());
			mBox = obb.mMinBox;
		}
		break;
	}

	// encode mesh
	origBox = mBox;
	encodeMesh();
}