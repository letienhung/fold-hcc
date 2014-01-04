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

	// special cases
	if (ns.isEmpty()) return NULL;
	if (ns.size() == 1) return ns[0];

	MeshMerger mm;
	foreach (FdNode* n, ns)
	{
		mm.addMesh(n->mMesh.data());
		removeNode(n->mID);
	}

	return addNode(mm.getMesh(), 0); 
}


FdNode* FdGraph::addNode( SurfaceMeshModel* mesh, int method )
{
	// box type
	Geom::Box box;
	QVector<Vector3> points = MeshHelper::getMeshVertices(mesh);
	if (method == 0){
		Geom::AABB aabb(points);
		box = aabb.box();
	}else{
		Geom::MinOBB obb(points, true);
		box = obb.mMinBox;
	}
	int box_type = box.getType(5);

	// create node depends on box type
	FdNode* node;
	if (box_type == Geom::Box::ROD)	
		node = new RodNode(MeshPtr(mesh), box);
	else node = new PatchNode(MeshPtr(mesh), box);
	
	Graph::addNode(node);
	node->mID = mesh->name;

	return node;
}


QVector<FdNode*> FdGraph::split( FdNode* fn, Geom::Plane& plane, double thr )
{
	QVector<FdNode*> splitted;

	// cut point along skeleton
	int aid = fn->mBox.getClosestAxisId(plane.Normal);
	Vector3 cutPoint = plane.getProjection(fn->mBox.Center);
	Vector3 cutCoord = fn->mBox.getCoordinates(cutPoint);
	double cp = cutCoord[aid];

	// no cut: cut point is too close to the end
	if (cp + 1 < thr || 1 - cp < thr)	return splitted;

	// split box
	Geom::Box box1, box2;
	fn->mBox.split(aid, cp, box1, box2);

	// split mesh
	Geom::Box cbox1 = box1; cbox1.Extent *= 1.001;
	Geom::Box cbox2 = box2; cbox2.Extent *= 1.001;
	SurfaceMeshModel* mesh1 = MeshBoolean::cork(fn->mMesh.data(), cbox2, MeshBoolean::DIFF);
	SurfaceMeshModel* mesh2 = MeshBoolean::cork(fn->mMesh.data(), cbox1, MeshBoolean::DIFF);
	mesh1->name = fn->mID + "_1";
	mesh2->name = fn->mID + "_2";

	// create new nodes
	FdNode *node1, *node2;
	MeshPtr meshPtr1(mesh1), meshPtr2(mesh2);
	if (fn->mType == FdNode::ROD)
	{
		node1 = new RodNode(meshPtr1, box1);
		node2 = new RodNode(meshPtr2, box2);
	}
	else
	{
		node1 = new PatchNode(meshPtr1, box1);
		node2 = new PatchNode(meshPtr2, box2);
	}
	node1->mID = mesh1->name; 
	node2->mID = mesh2->name; 

	// replace nodes
	removeNode(fn->mID);
	Structure::Graph::addNode(node1);
	Structure::Graph::addNode(node2);

	splitted.push_back(node1);
	splitted.push_back(node2);
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