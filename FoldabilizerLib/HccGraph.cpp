#include "HccGraph.h"
#include "Numeric.h"
#include "IntrBoxBox.h"
#include "xmlWriter.h"
#include <fstream>

HccGraph::HccGraph()
{
	isDraw = false;
}

HccGraph::HccGraph(QString fname)
{
	if(!loadHCC(fname))
		qDebug()<<"Failed to load HCC file!\n";

	isDraw = false;
}

HccGraph::~HccGraph()
{
    clear();
}

void HccGraph::clear()
{
	foreach(Node* n, nodes) delete n;
	foreach(Link* l, links) delete l;

	nodes.clear();
	links.clear();
}

void HccGraph::addNode(Node* node)
{
    nodes.push_back(node);
}

void HccGraph::addLink( Link* link)
{
	links.push_back(link);
}

void HccGraph::addLink( Node* n0, Node* n1 )
{
	links.push_back(new Link(n0, n1));
}

void HccGraph::removeNode( QString nodeID )
{
	// remove incident links
	foreach(Link* l, links)	{
		if (l->hasNode(nodeID))
			removeLink(l);
	}

	// find
	int idx = -1;
	for (int i = 0; i < nodes.size(); i++)	{
		if (nodes[i]->mID == nodeID) {
			idx = i;
			break;
		}
	}
	if (idx == -1) return;

	// deallocate and remove
	delete nodes[idx];
	nodes.remove(idx);
}

void HccGraph::removeLink( Link* link )
{
	// find and remove
	for (int i = 0; i < links.size(); i++)	{
		if (links[i] == link) {
			links.remove(i);
			break;
		}
	}

	// deallocate
	delete link;
}

Node* HccGraph::getNode(QString id)
{
	foreach(Node* n, nodes)
		if(n->mID == id) return n;

	return NULL;
}

Node* HccGraph::getNode( int idx )
{
	if (idx >= 0 && idx < nbNodes())
		return this->nodes[idx];
	else
		return NULL;
}

Link* HccGraph::getLink( QString nid1, QString nid2 )
{
	foreach(Link* l, links)	{
		if (l->hasNode(nid1) && l->hasNode(nid2))
			return l;
	}

	return NULL;
}

QVector<Link*> HccGraph::getLinks( QString nodeID )
{
	QVector<Link*> ls;
	foreach(Link* l, links){
		if (l->hasNode(nodeID)) ls.push_back(l);
	}

	return ls;
}

void HccGraph::computeAabb()
{
	if (nodes.isEmpty())
	{
		center = Vector3(0, 0, 0);
		radius = 1;
	}
	else
	{
		bbmin = Point( FLT_MAX,  FLT_MAX,  FLT_MAX);
		bbmax = Point(-FLT_MAX, -FLT_MAX, -FLT_MAX);	

		foreach(Node* n, nodes)	{
			QVector<Point> conners = n->mBox.getConnerPoints();
			foreach(Point p, conners) {
				bbmin = minimize(bbmin, p);
				bbmax = maximize(bbmax, p);
			}		
		}

		center = (bbmin + bbmax) * 0.5f;
		radius = (bbmax - bbmin).norm() * 0.5f;
	}
}

double HccGraph::getAabbVolume()
{
	this->computeAabb();
	Vector3 diagonal = bbmax - bbmin;
	return diagonal[0] * diagonal[1] * diagonal[2];
}


void HccGraph::restoreConfiguration()
{
	// initial
	if(isEmpty()) return;
	this->resetTags();

	// fix hinges
	// starting from base node
	QQueue<Node*> activeNodes;
	Node* base_node = this->getBaseNode(); 
	if (!base_node) base_node = nodes[0];
	base_node->isFixed = true;
	activeNodes.enqueue(base_node);

	while (!activeNodes.isEmpty())
	{
		Node* anode = activeNodes.dequeue();
		foreach(Link* l, getLinks(anode->mID))
		{
			if (l->fix()) 
			{
				activeNodes.enqueue(l->otherNode(anode->mID));
			}
		}
	}
}

bool HccGraph::isEmpty()
{
	return nodes.isEmpty();
}

void HccGraph::resetTags()
{
	foreach(Node* n, nodes) n->isFixed = false;
	foreach(Link* l, links) l->isFixed = false;
}

int HccGraph::nbNodes()
{
	return nodes.size();
}

int HccGraph::nbLinks()
{
	return links.size();
}

Node* HccGraph::getBaseNode()
{
	foreach(Node* n, nodes)
		if(n->mID.contains("_base"))
			return n;

	return NULL;
}

double HccGraph::getMtlVolume()
{
	double volume = 0;
	foreach (Node* n, nodes) volume += n->mBox.getVolume();
	return volume;
}

GraphState HccGraph::getState()
{
	GraphState state;

	foreach(Node* n, nodes) 
		state.node_scale_factor.push_back(n->scaleFactor);

	foreach(Link* l, links)
	{
		state.active_hinge_id.push_back(l->getActiveHingeId());
		state.hinge_angle.push_back(l->activeHinge()->angle);
	}

	return state;
}

void HccGraph::setState( GraphState &state )
{
	for (int i = 0; i < nodes.size(); i++)
	{
		nodes[i]->scale(state.node_scale_factor[i]);
	}

	for (int i = 0; i < links.size(); i++)
	{
		links[i]->setActiveHingeId(state.active_hinge_id[i]);
		links[i]->activeHinge()->angle = state.hinge_angle[i];
	}
}

QVector<Node*> HccGraph::getHotNodes(bool hot)
{
	QVector<Node*> hotNodes;
	foreach (Node* n, nodes)
		if (n->isHot == hot) hotNodes.push_back(n);

	return hotNodes;
}

QVector<Link*> HccGraph::getHotLinks(bool hot)
{
	QVector<Link*> hotLinks;
	foreach(Link* l, links)
		if (l->isHot == hot) hotLinks.push_back(l);

	return hotLinks;
}

Box HccGraph::getAabbBox()
{
	return Box(center, XYZ(), (bbmax-bbmin)/2);
}

void HccGraph::saveAsObj()
{
	std::ofstream outF("chair.obj", std::ios::out);

	if (!outF) return;


	foreach(Node* n, nodes)
	{
		foreach(Vector3 v, n->mBox.getConnerPoints())
			outF << "v " << v.x() <<" " << v.y() <<" " << v.z() << std::endl;
	}

	int v_offset = 1;
	for (int n = 0; n < this->nbNodes(); n++)
	{
		for (int i = 0; i < 12; i++)
		{
			outF << "f";
			for (int j = 0; j < 3; j++)
				outF<<" " << Box::TRI_FACE[i][j] + v_offset;

			outF << std::endl;
		}
		v_offset += 8;
	}

	outF.close();
}

bool HccGraph::isHingable( Link* link )
{
	GraphState backupState = this->getState();

	

	this->setState(backupState);

	return true;
}

void HccGraph::updateHingeScale()
{
	foreach(Link* l, links)
		l->setHingeScale(this->radius / 10);
}

void HccGraph::hotAnalyze()
{
	// reset hot tags
	foreach(Node* n, nodes) n->isHot = false;
	foreach(Link* l, links) l->isHot = false;

	// get planes of the shrunk AABB
	this->computeAabb();
	Box aabbBox = getAabbBox();
	aabbBox.scale(0.99);
	QVector<Plane> aabbFaces = aabbBox.getFacePlanes();

	// test if a node falls in both sides of planes
	foreach(Node* n, nodes)	
	{
		QVector<Vector3> nodePnts = n->mBox.getConnerPoints();
		for (int i = 0; i < aabbFaces.size(); i++)
		{
			if (!aabbFaces[i].onSameSide(nodePnts))
			{
				// hot node
				n->isHot = true;

				// hot links
				foreach(Link *l, this->getLinks(n->mID))
				{
					if (1)// TO Do: a link is hot only if it can still move
						l->isHot = true;
				}

				break;
			}
		}
	}
}


void HccGraph::detectHinges( bool ee /*= true*/, bool ef /*= true*/, bool ff /*= true*/ )
{
	foreach (Link* l, links)
		l->detectHinges(ee, ef, ff);

	this->updateHingeScale();
}

bool HccGraph::detectCollision()
{
	// clear collision record
	foreach(Node* n, nodes) n->collisionList.clear();

	// rough detection
	double s = 0.99;

	// detect collision between each pair of cuboids
	bool collide = false;
	for (int i = 0; i < nbNodes(); i++)
	for (int j = i + 1; j < nbNodes(); j++)
	{
		if (Geom::IntrBoxBox::test(nodes[i]->mBox, nodes[j]->mBox, s))
		{
			collide = true;

			// keep record
			nodes[i]->collisionList.push_back(nodes[j]);
			nodes[j]->collisionList.push_back(nodes[i]);
		}
	}

	return collide;
}

void HccGraph::draw()
{
	foreach(Link *l, links) l->draw();

	foreach(Node* n, nodes) n->drawDebug();

	// nodes are draw in transparency
	// draw nodes at the very end to show other contents
	foreach(Node *n, nodes) n->draw();
}

bool HccGraph::loadHCC(QString fname)
{
	// validate file
	QFile file(fname);
	file.open(QIODevice::ReadOnly);

	if(!file.isOpen()) return false;

	QDomDocument doc;
	if (!doc.setContent(&file)) return false;

	QDomElement root = doc.documentElement();
	if(root.tagName() != "Graph")
		return false;

	// clear current graph
	this->clear();

	// parse the file
	QDomNode node;
	QDomNode node_2;

	node = root.firstChild();
	int nBox = node.toElement().attribute("BoxAmount").toInt();
	int nLink = node.toElement().attribute("LinkAmount").toInt();

	// domNode of boxes
	node = node.nextSibling();
	{
		node_2 = node.toElement().firstChild();

		while(!node_2.isNull())
		{
			QString id = node_2.toElement().attribute("ID");
			Point center = Point(node_2.toElement().attribute("CenterX").toDouble(),
				node_2.toElement().attribute("CenterY").toDouble(),
				node_2.toElement().attribute("CenterZ").toDouble());

			QVector<Vector3> axis;
			axis.push_back(Vector3(node_2.toElement().attribute("Axis0X").toDouble(),
				node_2.toElement().attribute("Axis0Y").toDouble(),
				node_2.toElement().attribute("Axis0Z").toDouble()));

			axis.push_back(Vector3(node_2.toElement().attribute("Axis1X").toDouble(),
				node_2.toElement().attribute("Axis1Y").toDouble(),
				node_2.toElement().attribute("Axis1Z").toDouble()));

			axis.push_back(Vector3(node_2.toElement().attribute("Axis2X").toDouble(),
				node_2.toElement().attribute("Axis2Y").toDouble(),
				node_2.toElement().attribute("Axis2Z").toDouble()));

			Vector3 ext = Vector3(node_2.toElement().attribute("ExtentX").toDouble(),
				node_2.toElement().attribute("ExtentY").toDouble(),
				node_2.toElement().attribute("ExtentZ").toDouble());

			Box box(center, axis, ext);	 
			Node* curNode = new Node(box, id);
			this->addNode(curNode);

			node_2 = node_2.nextSibling();
		}
	}

	// domNode of links
	node = node.nextSibling();
	{
		node_2 = node.toElement().firstChild();

		while(!node_2.isNull())
		{
			QString box1ID = node_2.toElement().attribute("Box1ID");
			QString box2ID = node_2.toElement().attribute("Box2ID");

			Vector3 from = Vector3(node_2.toElement().attribute("CenterX").toDouble(),
				node_2.toElement().attribute("CenterY").toDouble(),
				node_2.toElement().attribute("CenterZ").toDouble());

			Vector3 to = Vector3(node_2.toElement().attribute("ToX").toDouble(),
				node_2.toElement().attribute("ToY").toDouble(),
				node_2.toElement().attribute("ToZ").toDouble());

			//this->hccGraph->addLink(new Link(getNode(box1ID), getNode(box2ID), (from+to)/2, to-from));

			node_2 = node_2.nextSibling();
		}
	}

	// update AABB
	this->computeAabb();
	return true;
}

bool HccGraph::saveHCC(QString fname)
{
	if(fname.isNull())
		return false;

	QFile file(fname);//.toStdString().c_str());
	file.open(QIODevice::WriteOnly);
	if(!file.isOpen()) return false;
	XmlWriter xw(&file);

	xw.setAutoNewLine(true);	
	xw.writeRaw("\<!--LCC Version = \"1.0\"--\>\n\<Graph\>");
	xw.newLine();

	AttrMap amount_attrs;
	amount_attrs.insert("BoxAmount", QString::number(nodes.size()));
	amount_attrs.insert("LinkAmount", QString::number(links.size()));
	xw.writeAtomTag("Amount", amount_attrs);

	xw.writeOpenTag("BoxPool");
	for(int i=0; i < nodes.size(); i++)
	{
		AttrMap box_attrs;
		box_attrs.insert("ID", nodes[i]->mID);
		box_attrs.insert("CenterX", QString::number(nodes[i]->mBox.Center.x()));
		box_attrs.insert("CenterY", QString::number(nodes[i]->mBox.Center.y()));
		box_attrs.insert("CenterZ", QString::number(nodes[i]->mBox.Center.z()));
		box_attrs.insert("Axis0X", QString::number(nodes[i]->mBox.Axis[0].x()));
		box_attrs.insert("Axis0Y", QString::number(nodes[i]->mBox.Axis[0].y()));
		box_attrs.insert("Axis0Z", QString::number(nodes[i]->mBox.Axis[0].z()));
		box_attrs.insert("Axis1X", QString::number(nodes[i]->mBox.Axis[1].x()));
		box_attrs.insert("Axis1Y", QString::number(nodes[i]->mBox.Axis[1].y()));
		box_attrs.insert("Axis1Z", QString::number(nodes[i]->mBox.Axis[1].z()));
		box_attrs.insert("Axis2X", QString::number(nodes[i]->mBox.Axis[2].x()));
		box_attrs.insert("Axis2Y", QString::number(nodes[i]->mBox.Axis[2].y()));
		box_attrs.insert("Axis2Z", QString::number(nodes[i]->mBox.Axis[2].z()));
		box_attrs.insert("ExtentX", QString::number(nodes[i]->mBox.Extent.x()));
		box_attrs.insert("ExtentY", QString::number(nodes[i]->mBox.Extent.y()));
		box_attrs.insert("ExtentZ", QString::number(nodes[i]->mBox.Extent.z()));
		xw.writeAtomTag("Box", box_attrs);
	}
	xw.writeCloseTag("BoxPool");

	xw.writeOpenTag("LinkPool");
	for(int i=0; i < links.size(); i++)
	{
		AttrMap link_attrs;
		link_attrs.insert("Box1ID", links[i]->node1->mID);
		link_attrs.insert("Box1ID", links[i]->node2->mID);
		//link_attrs.insert("CenterX", QString::number(links[i]->center.x()));
		//link_attrs.insert("CenterY ", QString::number(links[i]->center.y()));
		//link_attrs.insert("CenterZ", QString::number(links[i]->center.z()));
		//link_attrs.insert("ToX ", QString::number(links[i]->center.x()+links[i]->axis.x()));
		//link_attrs.insert("ToY ", QString::number(links[i]->center.y()+links[i]->axis.y()));
		//link_attrs.insert("ToZ", QString::number(links[i]->center.z()+links[i]->axis.z()));
		xw.writeAtomTag("Link", link_attrs);
	}
	xw.writeCloseTag("LinkPool");

	xw.writeRaw("\<\/Graph\>");
	file.close();

	return true;
}
