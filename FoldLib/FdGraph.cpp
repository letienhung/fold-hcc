#include "FdGraph.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "XmlWriter.h"
#include "FdUtility.h"

#include "RodNode.h"
#include "PatchNode.h"

FdGraph::FdGraph()
{
	showAABB = false;
	path = "";
}

QVector<FdNode*> FdGraph::getFdNodes()
{
	QVector<FdNode*> fdns;
	foreach(Structure::Node* n, nodes)
		fdns.push_back((FdNode*)n);
	
	return fdns;
}

void FdGraph::saveToFile(QString fname)
{
	QFile file(fname);
	if (!file.open(QIODevice::WriteOnly)) return;

	// scaffold
	XmlWriter xw(&file);
	xw.setAutoNewLine(true);	
	xw.writeRaw("\<!--?xml Version = \"1.0\"?--\>\n");
	xw.writeOpenTag("document");
	{
		// nodes
		xw.writeTaggedString("cN", QString::number(nbNodes()));
		foreach(FdNode* node, getFdNodes())
		{
			node->writeToXml(xw);
		}

		// links
		xw.writeTaggedString("cL", QString::number(nbLinks()));
	}
	xw.writeCloseTag("document");
	file.close();

	// meshes
	QString meshesFolder = "meshes";
	QFileInfo fileInfo(fname);
	QDir graphDir( fileInfo.absolutePath());
	graphDir.mkdir(meshesFolder);
	meshesFolder =  graphDir.path() + "/" + meshesFolder;
	foreach(FdNode* node, getFdNodes())
	{
		saveOBJ(node->mMesh.data(), meshesFolder + '/' + node->id + ".obj");
	}
}

void FdGraph::loadFromFile(QString fname)
{
	clear();
	path = fname;

	// open the file
	QFileInfo finfo(fname);
	QString meshFolder = finfo.path() + "/meshes";
	QFile file(fname);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	// cast using DOM
	QDomDocument doc;
	if (!doc.setContent(&file, false)) {
		file.close();
		return;
	}
	file.close();

	QDomNodeList nodeList = doc.firstChildElement("document").elementsByTagName("node");
	for (int i = 0; i < nodeList.size(); i++)
	{
		QDomNode node = nodeList.at(i);

		// scaffold
		QString nid = node.firstChildElement("ID").text();
		int ntype = node.firstChildElement("type").text().toInt();
		Geom::Box box = getBox(node.firstChildElement("box"));

		// mesh
		QString mesh_fname = meshFolder + "/" + nid + ".obj";
		SurfaceMeshModel* mesh(new SurfaceMeshModel(mesh_fname, nid));
		if(mesh->read( qPrintable(mesh_fname) ))
		{
			mesh->update_face_normals();
			mesh->update_vertex_normals();
			mesh->updateBoundingBox();
		}

		// create new node
		FdNode* new_node;
		MeshPtr m(mesh);
		if(ntype == FdNode::ROD)
			new_node = new RodNode(m, box);
		else
			new_node = new PatchNode(m, box);

		addNode(new_node);
	}
}



Geom::AABB FdGraph::computeAABB()
{
	Geom::AABB aabb;
	foreach (FdNode* n, getFdNodes())
	{
		aabb.add(n->computeAABB());
	}

	// in case the graph is empty
	aabb.validate();

	return aabb;
}

void FdGraph::draw()
{
	Structure::Graph::draw();

	// draw aabb
	if (showAABB)
	{
		Geom::AABB aabb = computeAABB();
		aabb.box().drawWireframe(2.0, Qt::cyan);
	}
}
