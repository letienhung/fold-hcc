#include "FdNode.h"
#include "CustomDrawObjects.h"
#include "Numeric.h"
#include "FdUtility.h"

FdNode::FdNode( SurfaceMeshModel *m, Geom::Box &b )
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
	foreach(Vector3 p, getMeshVertices(mMesh))
		meshCoords.push_back(origBox.getCoordinates(p));
}

void FdNode::deformMesh()
{
	Surface_mesh::Vertex_property<Point> points = mMesh->vertex_property<Point>("v:point");

//#pragma omp parallel for
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

