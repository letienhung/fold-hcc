#include "Scaffold.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "XmlWriter.h"
#include "MeshMerger.h"
#include "MeshHelper.h"
#include "MeshBoolean.h"

#include "RodNode.h"
#include "PatchNode.h"
#include "BundleNode.h"
#include "ScaffLink.h"

#include "AABB.h"
#include "MinOBB.h"
#include "CustomDrawObjects.h"


Scaffold::Scaffold(QString id /*= ""*/)
	:Graph(id)
{
	showAABB = false;
	path = "";
}

Scaffold::Scaffold( Scaffold& other )
	:Graph(other)
{
	path = other.path + "_cloned";
	showAABB = false;
}

Scaffold::Scaffold(QVector<Scaffold*> scaffs, QString baseMid,
	QMap<QString, QSet<int> >& masterScaffMap)
{
	// combined tags
	QVector<bool> scaffCombined(scaffs.size(), false);
	QMap<QString, bool> masterCombined;
	for(QString mid : masterScaffMap.keys())
		masterCombined[mid] = false;

	// copy the base master from any scaffold
	int sid = masterScaffMap[baseMid].toList().front();
	Structure::Graph::addNode(scaffs[sid]->getNode(baseMid)->clone());
	masterCombined[baseMid] = true;

	// prorogation
	QQueue<QString> activeMids; activeMids.enqueue(baseMid);
	while (!activeMids.isEmpty())
	{
		// set current to an active master
		QString fixed_mid = activeMids.dequeue();
		PatchNode* fixedMaster = (PatchNode*)this->getNode(fixed_mid);
		Vector3 fixedMasterPos = fixedMaster->center();

		// combine scaffs containing current master
		for(int scaff_id : masterScaffMap[fixed_mid])
		{
			// skip combined scaff
			if (scaffCombined[scaff_id]) continue;

			// translate scaff to match the master
			Scaffold* scaff = scaffs[scaff_id];
			Vector3 scaffMastertPos = ((PatchNode*)scaff->getNode(fixed_mid))->center();
			scaff->translate(fixedMasterPos - scaffMastertPos);

			// combine parts from decomposition
			for(Structure::Node* n : scaff->nodes)
			{
				// master nodes
				if (n->hasTag(MASTER_TAG))
				{
					if (!masterCombined[n->mID])
					{
						// combine unvisited masters
						Structure::Graph::addNode(n->clone());
						masterCombined[n->mID] = true;

						// store as active
						activeMids.enqueue(n->mID);
					}
				}
				// clone slave nodes
				else
				{
					Structure::Graph::addNode(n->clone());
				}
			}

			// update tag
			scaffCombined[scaff_id] = true;
		}
	}
}


Scaffold::~Scaffold()
{

}

Structure::Graph* Scaffold::clone()
{
	return new Scaffold(*this);
}

ScaffLink* Scaffold::addLink( ScaffNode* n1, ScaffNode* n2 )
{
	ScaffLink* new_link = new ScaffLink(n1, n2);
	Graph::addLink(new_link);

	return new_link;
}


QVector<ScaffNode*> Scaffold::getScaffNodes()
{
	QVector<ScaffNode*> fdns;
	for (Structure::Node* n : nodes)
		fdns.push_back((ScaffNode*)n);
	
	return fdns;
}

ScaffNode* Scaffold::getScaffNode(QString id)
{
	Structure::Node* n = getNode(id);
	if (n != nullptr) 
		return (ScaffNode*)n;
	else 
		return nullptr;
}


void Scaffold::exportMesh(QString fname)
{
	QFile file(fname);

	// Open for writing
	if (!file.open(QIODevice::Append | QIODevice::Text)) return;
	
	int v_offset = 0;
	QVector<ScaffNode*> nodes = getScaffNodes();
	for (ScaffNode *n : nodes){
		n->deformMesh();
		n->exportMesh(file, v_offset);
	}
	file.close();
}

void Scaffold::saveToFile(QString fname)
{
	QFile file(fname);
	if (!file.open(QIODevice::WriteOnly)) return;

	// scaffold
	XmlWriter xw(&file);
	xw.setAutoNewLine(true);	
	xw.writeRaw("<!--?xml Version = \"1.0\"?-->\n");
	xw.writeOpenTag("document");
	{
		// nodes
		xw.writeTaggedString("cN", QString::number(nbNodes()));
		for (ScaffNode* node : getScaffNodes())
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
	for (ScaffNode* node : getScaffNodes())
	{
		MeshHelper::saveOBJ(node->mMesh.data(), meshesFolder + '/' + node->mID + ".obj");
	}
}

void Scaffold::loadFromFile(QString fname)
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
		ScaffNode* new_node;
		MeshPtr m(mesh);
		QString id = m->name;
		if(ntype == ScaffNode::ROD)
			new_node = new RodNode(id, box, m);
		else
			new_node = new PatchNode(id, box, m);

		Graph::addNode(new_node);
	}
}


Geom::AABB Scaffold::computeAABB()
{
	Geom::AABB aabb;
	for (ScaffNode* n : getScaffNodes())
	{
		aabb.add(n->computeAABB());
	}

	// in case the graph is empty
	aabb.validate();

	return aabb;
}


void Scaffold::drawAABB()
{
	Geom::Box box = computeAABB().box();

	if (properties.contains("pushAId"))
	{
		int aid = properties["pushAId"].toInt();
		Geom::Rectangle patch1 = box.getCrossSection(aid, 1);
		Geom::Rectangle patch2 = box.getCrossSection(aid, -1);

		patch1.drawEdges(2.0, Qt::yellow);
		patch2.drawEdges(2.0, Qt::yellow);

		LineSegments ls(2.0);
		for (Geom::Segment seg : box.getEdgeSegments(aid))
			ls.addLine(seg.P0, seg.P1, Qt::cyan);
		ls.draw();
	}
	else
	{
		box.drawWireframe(2.0, Qt::cyan);
	}
}


void Scaffold::draw()
{
	Structure::Graph::draw();

	// draw aabb
	if (showAABB) drawAABB();

	// debug
	drawDebug();
}

ScaffNode* Scaffold::wrapAsBundleNode( QVector<QString> nids, Vector3 v )
{
	// retrieve all nodes
	QVector<ScaffNode*> ns;
	for (QString nid : nids)
	{
		Structure::Node* n = getNode(nid);
		if (n) ns.push_back((ScaffNode*)n);
	}

	// trivial cases
	if (ns.isEmpty()) return nullptr;
	if (ns.size() == 1) return ns[0];

	// merge into a bundle node
	QVector<ScaffNode*> subNodes;
	for (ScaffNode* n : ns)	subNodes += n->getSubNodes();
	QString bid = getBundleName(subNodes);
	Geom::Box box = getBundleBox(subNodes);
	BundleNode* bundleNode = new BundleNode(bid, box, subNodes, v); 
	Structure::Graph::addNode(bundleNode);

	// remove original nodes
	for (ScaffNode* n : ns) removeNode(n->mID);

	return bundleNode;
}


ScaffNode* Scaffold::addNode( MeshPtr mesh, BOX_FIT_METHOD method )
{
	// fit box
	QVector<Vector3> points = MeshHelper::getMeshVertices(mesh.data());
	Geom::Box box = fitBox(points, method);

	return addNode(mesh, box);
}

ScaffNode* Scaffold::addNode(MeshPtr mesh, Geom::Box& box)
{
	// create node depends on box type
	int box_type = box.getType(5);

	ScaffNode* node;
	QString id = mesh->name;
	if (box_type == Geom::Box::ROD)	
		node = new RodNode(id, box, mesh);
	else node = new PatchNode(id, box, mesh);

	// add to graph and set id
	Graph::addNode(node);
	node->mID = mesh->name;

	return node;
}

QVector<ScaffNode*> Scaffold::split( QString nid, Geom::Plane& plane)
{
	return split(nid, QVector<Geom::Plane>() << plane);
}

// if cut planes intersect with the part, the part is split and replaced by chopped parts
// otherwise the original part remains
// the return value could either be the original part or the chopped fragment parts
QVector<ScaffNode*> Scaffold::split( QString nid, QVector<Geom::Plane>& planes )
{
	ScaffNode* fn = (ScaffNode*) getNode(nid);

	// in case
	QVector<ScaffNode*> chopped;
	chopped << fn;
	if (planes.isEmpty()) return chopped;
	if (!fn) return chopped;

	// sort cut planes 
	int aid = fn->mBox.getAxisId(planes.front().Normal);
	Geom::Segment sklt = fn->mBox.getSkeleton(aid);
	QMap<double, Geom::Plane> distPlaneMap;
	for (Geom::Plane plane : planes)
	{
		//// skip a plane if it doesn't intersect the node
		//if (relationWithPlane(fn, plane, 0.1) != ISCT_PLANE) continue;

		Vector3 cutPoint = plane.getIntersection(sklt);
		double dist = (cutPoint - sklt.P0).norm();
		distPlaneMap[dist] = plane;
	}
	QVector<Geom::Plane> sortedPlanes = distPlaneMap.values().toVector();
	if (sortedPlanes.isEmpty()) return chopped;

	// cut planes DO intersect the part, do splitting
	chopped.clear();

	// the front end
	Geom::Plane frontPlane = sortedPlanes.front();
	if (frontPlane.whichSide(sklt.P0) < 0) frontPlane.flip();
	chopped << fn->cloneChopped(frontPlane);

	// the middle section bounded by two planes
	for (int i = 0; i < sortedPlanes.size()-1; i++)
	{
		chopped << fn->cloneChopped(sortedPlanes[i], sortedPlanes[i+1]);
	}

	// the back end
	Geom::Plane backPlane = sortedPlanes.back();
	if (backPlane.whichSide(sklt.P1) < 0) backPlane.flip();
	chopped << fn->cloneChopped(backPlane);

	// change node id and add to graph
	QString origMId = nid;
	if (fn->properties.contains(SPLIT_ORIG))
		origMId = fn->properties[SPLIT_ORIG].value<QString>();
	for (int i = 0; i < chopped.size(); i++)
	{
		chopped[i]->mID = nid + "sp" + QString::number(i);
		chopped[i]->properties[SPLIT_ORIG] = origMId;
		Structure::Graph::addNode(chopped[i]);
	}

	// inherits color
	for (int i = 0; i < chopped.size(); i++)
	{
		if (i % 2 == 0)
			chopped[i]->mColor = fn->mColor;
		else
			chopped[i]->mColor = fn->mColor.lighter(120);
	}

	// remove the original node
	removeNode(nid);

	return chopped;
}

void Scaffold::showCuboids( bool show )
{
	for (ScaffNode* n : getScaffNodes())
		n->setShowCuboid(show);
}

void Scaffold::showScaffold( bool show )
{
	for (ScaffNode* n : getScaffNodes())
		n->setShowScaffold(show);
}

void Scaffold::showMeshes( bool show )
{
	for (ScaffNode* n : getScaffNodes())
		n->setShowMesh(show);
}

void Scaffold::changeNodeType( ScaffNode* n )
{
	// create new node
	Structure::Node* new_node;
	if (n->mType == ScaffNode::PATCH)
		new_node =new RodNode(n->mID, n->mBox, n->mMesh);
	else
		new_node = new PatchNode(n->mID, n->mBox, n->mMesh);

	// replace
	replaceNode(n, new_node);
}

PatchNode* Scaffold::changeRodToPatch( RodNode* rn, Vector3 v )
{
	PatchNode* pn = new PatchNode(rn,  v);
	replaceNode(rn, pn);
	return pn;
}

void Scaffold::restoreConfiguration()
{ 
	QQueue<ScaffNode*> activeNodes;
	for (ScaffNode* fn : getScaffNodes())
		if (fn->hasTag(FIXED_NODE_TAG)) 
			activeNodes.enqueue(fn);

	while (!activeNodes.isEmpty())
	{
		ScaffNode* anode = activeNodes.dequeue();
		for (Structure::Link* l : getLinks(anode->mID))
		{
			ScaffLink* fl = (ScaffLink*)l;
			if (!fl->hasTag(ACTIVE_LINK_TAG)) continue;

			if (fl->fix()) 
			{
				Structure::Node* other_node = l->getNodeOther(anode->mID);
				activeNodes.enqueue((ScaffNode*)other_node);
			}
		}
	}
}

void Scaffold::translate(Vector3 v, bool withMesh)
{
	for (ScaffNode* n : getScaffNodes())
	{
		n->translate(v);

		if (withMesh)
			n->deformMesh();
	}
}

void Scaffold::drawDebug()
{
	// debug points
	if (properties.contains(DEBUG_POINTS))
	{
		QVector<Vector3> debugPoints = properties[DEBUG_POINTS].value<QVector<Vector3> >();
		PointSoup ps;
		for (Vector3 p : debugPoints) ps.addPoint(p);
		ps.draw();
	}

	// debug segments
	if (properties.contains(DEBUG_SEGS))
	{
		QVector<Geom::Segment> debugSegs = properties[DEBUG_SEGS].value< QVector<Geom::Segment> >();
		for (Geom::Segment seg : debugSegs)
		{
			seg.draw();
		}
	}

	// debug boxes
	if (properties.contains(DEBUG_BOXES))
	{
		QColor color = Qt::green; color.setAlphaF(1.0);
		QVector<Geom::Box> debugBoxes = properties[DEBUG_BOXES].value<QVector<Geom::Box> >();
		for (Geom::Box box : debugBoxes)
		{
			box.drawWireframe(2.0, color);
		}
	}

	// debug planes
	if (properties.contains(DEBUG_PLANES))
	{
		QVector<Geom::Plane> planes = properties[DEBUG_PLANES].value<QVector<Geom::Plane> >();
		for (Geom::Plane p : planes)
		{
			p.draw();
		}
	}

	// debug rects
	if (properties.contains(DEBUG_RECTS))
	{
		QVector<Geom::Rectangle> regions = properties[DEBUG_RECTS].value< QVector<Geom::Rectangle> >();
		for (Geom::Rectangle rect : regions)
		{
			rect.drawEdges(2.0, Qt::blue);
		}
	}

	// scaffold
	if (properties.contains(DEBUG_SCAFFS))
	{
		QVector<Scaffold*> debugSs = properties[DEBUG_SCAFFS].value<QVector<Scaffold*> >();
		for (Scaffold* ds : debugSs)
		{
			if (ds) ds->draw();
		}
	}
}

void Scaffold::unwrapBundleNodes()
{
	foreach(Structure::Node* n, nodes)
	{
		if (n->hasTag(BUNDLE_TAG))
		{
			BundleNode* bnode = (BundleNode*)n;
			for (ScaffNode* cn : bnode->mNodes)
			{
				Structure::Node* cn_copy = cn->clone();
				Structure::Graph::addNode(cn_copy);
				if (bnode->hasTag(MASTER_TAG))
					cn_copy->addTag(MASTER_TAG);
			}

			removeNode(bnode->mID);
		}
	}
}

void Scaffold::hideEdgeRods()
{
	for (ScaffNode* n : getScaffNodes())
	{
		if (n->hasTag(EDGE_ROD_TAG))
		{
			n->isHidden = true;
		}
	}
}