#include "FdGraph.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "XmlWriter.h"
#include "MeshMerger.h"
#include "MeshHelper.h"
#include "MeshBoolean.h"

#include "RodNode.h"
#include "PatchNode.h"
#include "FdLink.h"

#include "AABB.h"
#include "MinOBB.h"
#include "FdUtility.h"


FdGraph::FdGraph(QString id /*= ""*/)
	:Graph(id)
{
	showAABB = false;
	path = "";
}

FdGraph::FdGraph( FdGraph& other )
	:Graph(other)
{
	path = other.path + "_cloned";
	showAABB = false;
}


Structure::Graph* FdGraph::clone()
{
	return new FdGraph(*this);
}


FdLink* FdGraph::addLink( FdNode* n1, FdNode* n2 )
{
	FdLink* new_link = new FdLink(n1, n2);
	Graph::addLink(new_link);

	return new_link;
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
			node->write(xw);
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
		MeshHelper::saveOBJ(node->mMesh.data(), meshesFolder + '/' + node->mID + ".obj");
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
		Geom::Box box;
		box.read(node.firstChildElement("box"));

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

		Graph::addNode(new_node);
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

FdNode* FdGraph::merge( QVector<QString> nids )
{
	// retrieve all nodes
	QVector<FdNode*> ns;
	foreach(QString nid, nids)
	{
		Structure::Node* n = getNode(nid);
		if (n) ns.push_back((FdNode*)n);
	}

	// trivial cases
	if (ns.isEmpty()) return NULL;
	if (ns.size() == 1) return ns[0];

	// merge
	MeshMerger mm;
	QVector<Vector3> boxPoints;
	foreach (FdNode* n, ns)
	{
		n->deformMesh();
		mm.addMesh(n->mMesh.data());

		boxPoints += n->mBox.getConnerPoints();

		removeNode(n->mID);
	}

	// fit box using box corners
	Geom::Box box = fitBox(boxPoints);

	return addNode(MeshPtr(mm.getMesh()), box); 
}


FdNode* FdGraph::addNode( MeshPtr mesh, int method )
{
	// fit box
	QVector<Vector3> points = MeshHelper::getMeshVertices(mesh.data());
	Geom::Box box = fitBox(points, method);

	return addNode(mesh, box);
}

FdNode* FdGraph::addNode(MeshPtr mesh, Geom::Box& box)
{
	// create node depends on box type
	int box_type = box.getType(5);

	FdNode* node;
	if (box_type == Geom::Box::ROD)	
		node = new RodNode(mesh, box);
	else node = new PatchNode(mesh, box);

	// add to graph and set id
	Graph::addNode(node);
	node->mID = mesh->name;

	return node;
}

QVector<FdNode*> FdGraph::split( FdNode* fn, Geom::Plane& plane)
{
	return split(fn, plane, plane.opposite());
}

// split by two planes: get two ends  <--|-|-->
QVector<FdNode*> FdGraph::split( FdNode* fn, Geom::Plane& plane1, Geom::Plane& plane2 )
{
	QVector<FdNode*> splitted;

	// positive side
	FdNode* node1 = fn->cloneChopped(plane1);
	if (node1)
	{
		QString id = fn->mID + QString("_%1").arg(splitted.size());
		node1->setStringId(id);
		Structure::Graph::addNode(node1);
		splitted.push_back(node1);
	}

	// negative side
	FdNode* node2 = fn->cloneChopped(plane2);
	if (node2)
	{
		QString id = fn->mID + QString("_%1").arg(splitted.size());
		node2->setStringId(id);
		Structure::Graph::addNode(node2);
		splitted.push_back(node2);
	}

	// remove the original node
	if (!splitted.isEmpty())
		removeNode(fn->mID);

	return splitted;
}

void FdGraph::showCuboids( bool show )
{
	foreach(FdNode* n, getFdNodes())
		n->showCuboids = show;
}

void FdGraph::showScaffold( bool show )
{
	foreach(FdNode* n, getFdNodes())
		n->showScaffold = show;
}

void FdGraph::showMeshes( bool show )
{
	foreach(FdNode* n, getFdNodes())
		n->showMesh = show;
}

void FdGraph::changeNodeType( FdNode* n )
{
	// create new node
	Structure::Node* new_node;
	if (n->mType == FdNode::PATCH)
		new_node =new RodNode(n->mMesh, n->mBox);
	else
		new_node = new PatchNode(n->mMesh, n->mBox);

	// replace
	replaceNode(n, new_node);
}


void FdGraph::restoreConfiguration()
{
	QQueue<FdNode*> activeNodes;
	foreach(FdNode* fn, getFdNodes())
	{
		if (fn->properties["fixed"].toBool())
		{
			activeNodes.enqueue(fn);
		}
	}

	while (!activeNodes.isEmpty())
	{
		FdNode* anode = activeNodes.dequeue();
		foreach(Structure::Link* l, getLinks(anode->mID))
		{
			FdLink* fl = (FdLink*)l;
			if (!fl->properties["active"].toBool()) continue;

			if (fl->fix()) 
			{
				Structure::Node* other_node = l->getNodeOther(anode->mID);
				activeNodes.enqueue((FdNode*)other_node);
			}
		}
	}
}

void FdGraph::normalize(double f)
{
	Geom::AABB aabb = computeAABB();
	Vector3 offset = aabb.center();

	foreach(FdNode* n, getFdNodes())
	{
		n->mBox.scaleAsPart(f, offset);
		n->deformMesh();
		n->createScaffold();
	}
}

void FdGraph::translate(Vector3 v)
{
	foreach(FdNode* n, getFdNodes())
	{
		n->mBox.translate(v);
		n->deformMesh();
		n->createScaffold();
	}
}