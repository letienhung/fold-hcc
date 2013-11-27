#include "Graph.h"
#include "xmlWriter.h"
#include "Numeric.h"

#include <fstream>

Graph::Graph()
{
	isDraw = false;
}

Graph::Graph(QString fname)
{
	if(!loadHCC(fname))
		qDebug()<<"Failed to load HCC file!\n";

	isDraw = false;
}

Graph::~Graph()
{
    clear();
}

void Graph::clear()
{
	foreach(Node* n, nodes) delete n;
	foreach(Link* l, links) delete l;

	nodes.clear();
	links.clear();
}

void Graph::addNode(Node* node)
{
    nodes.push_back(node);
}

void Graph::addLink( Link* link)
{
	links.push_back(link);
}

void Graph::removeNode( QString nodeID )
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

void Graph::removeLink( Link* link )
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

Node* Graph::getNode(QString id)
{
	foreach(Node* n, nodes)
		if(n->mID == id) return n;

	return NULL;
}

Node* Graph::getNode( int idx )
{
	if (idx >= 0 && idx < nbNodes())
		return this->nodes[idx];
	else
		return NULL;
}

Link* Graph::getLink( QString nid1, QString nid2 )
{
	foreach(Link* l, links)	{
		if (l->hasNode(nid1) && l->hasNode(nid2))
			return l;
	}

	return NULL;
}

QVector<Link*> Graph::getLinks( QString nodeID )
{
	QVector<Link*> ls;
	foreach(Link* l, links){
		if (l->hasNode(nodeID)) ls.push_back(l);
	}

	return ls;
}

bool Graph::loadHCC(QString fname)
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
		    nodes.push_back(curNode);

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
			
			//this->addLink(new Link(getNode(box1ID), getNode(box2ID), (from+to)/2, to-from));

			node_2 = node_2.nextSibling();
		}
	}

	// update AABB
	this->computeAabb();
	return true;
}

bool Graph::saveHCC(QString fname)
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

void Graph::draw()
{
	foreach(Link *l, links) l->draw();
	foreach(Node *n, nodes) n->draw();
}

void Graph::makeI()
{
	this->clear();
	QVector<Vector3> xyz = XYZ();

	Box tBox(Point(0.5, 6, 0), xyz, Vector3(0.5, 2, 0.5));
	Node* tNode = new Node(tBox, "top");

	Box bBox(Point(0.5, 2, 0), xyz, Vector3(0.5, 2, 0.5));
	Node* bNode = new Node(bBox, "bottom");

	this->addNode(tNode);
	this->addNode(bNode);
	this->addLink(new Link(tNode, bNode));

	this->computeAabb();
	this->updateHingeScale();
}

void Graph::makeL()
{
	this->clear();
	QVector<Vector3> xyz = XYZ();

	Box vBox(Point(-0.5, 2, 0), xyz, Vector3(0.5, 2, 2));
	Node* vNode = new Node(vBox, "vBox");

	Box hBox(Point(2, -0.5, 0), xyz, Vector3(2, 0.5, 2));
	Node* hNode = new Node(hBox, "hBox");

	this->addNode(vNode);
	this->addNode(hNode);

	this->addLink(new Link(vNode, hNode));

	this->computeAabb();
	this->updateHingeScale();
}

void Graph::makeT()
{
	this->clear();
	QVector<Vector3> xyz = XYZ();

	Box vBox(Point(0, -2, 0), xyz, Vector3(0.5, 2, 2));
	Node* vNode = new Node(vBox, "vBox");

	QVector<Vector3> rot_xyz;
	rot_xyz.push_back(Vector3(1, 0, 1));
	rot_xyz.push_back(Vector3(0, 1, 0));
	rot_xyz.push_back(Vector3(-1, 0, 1));

	Box hBox(Point(0, 0.5, 0), rot_xyz, Vector3(2, 0.5, 2));
	Node* hNode = new Node(hBox, "hBox_base");

	this->addNode(vNode);
	this->addNode(hNode);
	this->addLink(new Link(vNode, hNode));

	this->computeAabb();
	this->updateHingeScale();
}

void Graph::makeX()
{
	this->clear();
	QVector<Vector3> xyz = XYZ();

	Box vBox(Point(0, 0, 0.5), xyz, Vector3(0.5, 4, 0.5));
	Node* vNode = new Node(vBox, "vBox");

	Box hBox(Point(0, 0, -0.5), xyz, Vector3(4, 0.5, 0.5));
	Node* hNode = new Node(hBox, "hBox");

	this->addNode(vNode);
	this->addNode(hNode);

	this->addLink(new Link(vNode, hNode));

	this->computeAabb();
	this->updateHingeScale();
}

void Graph::makeSharp()
{
	this->clear();
	QVector<Vector3> xyz = XYZ();

	Node* vlNode = new Node(Box(Point(-2,  0,  0.5), xyz, Vector3(0.5, 4, 0.5)), "v-left_base"); 
	Node* vrNode = new Node(Box(Point( 2,  0,  0.5), xyz, Vector3(0.5, 4, 0.5)), "v-right");
	Node* htNode = new Node(Box(Point( 0,  2, -0.5), xyz, Vector3(4, 0.5, 0.5)), "h-top");
	Node* hbNode = new Node(Box(Point( 0, -2, -0.5), xyz, Vector3(4, 0.5, 0.5)), "h-bottom");

	addNode(vlNode); addNode(vrNode); addNode(htNode); addNode(hbNode);

	addLink(new Link(vlNode, hbNode));
	addLink(new Link(vrNode, hbNode));
	addLink(new Link(vlNode, htNode));
	addLink(new Link(vrNode, htNode));

	this->computeAabb();
	this->updateHingeScale();
}


void Graph::makeU( double uleft, double umid, double uright )
{
	this->clear();
	QVector<Vector3> xyz = XYZ();

	Box ltBox(Point(0.5, 3 * uleft, 0), xyz, Vector3(0.5, uleft, 0.5));
	Node* ltNode = new Node(ltBox, "left-top");

	Box lbBox(Point(0.5, uleft, 0), xyz, Vector3(0.5, uleft, 0.5));
	Node* lbNode = new Node(lbBox, "left-bottom");

	Box hBox(Point(umid, -0.5, 0), xyz, Vector3(umid, 0.5, 0.5));
	Node* hNode = new Node(hBox, "horizontal_base");

	Box rBox(Point(2 * umid - 0.5, uright, 0), xyz, Vector3(0.5, uright, 0.5));
	Node* rNode = new Node(rBox, "right");

	
	this->addNode(lbNode);
	this->addNode(ltNode);
	this->addNode(hNode);
	this->addNode(rNode);

	this->addLink(new Link(ltNode, lbNode));
	this->addLink(new Link(lbNode, hNode));
	Link* nailedLink = new Link(rNode, hNode);
	//nailedLink->isNailed = true;
	this->addLink(nailedLink);

	this->computeAabb();
	this->updateHingeScale();
}

void Graph::makeO()
{
	this->clear();
	QVector<Vector3> xyz = XYZ();

	Box lBox(Point(0.5, 2, 0), xyz, Vector3(0.5, 2, 0.5));
	Node* lNode = new Node(lBox, "left");

	Box rBox(Point(7.5, 2, 0), xyz, Vector3(0.5, 2, 0.5));
	Node* rNode = new Node(rBox, "right");	
	
	Box bBox(Point(4, -0.5, 0), xyz, Vector3(3, 0.5, 0.5));
	Node* bNode = new Node(bBox, "bottom_base");

	Box tBox(Point(4, 4.5, 0), xyz, Vector3(3, 0.5, 0.5));
	Node* tNode = new Node(tBox, "top");

	this->addNode(lNode);
	this->addNode(rNode);
	this->addNode(bNode);
	this->addNode(tNode);

	this->addLink(new Link(bNode, lNode));
	this->addLink(new Link(bNode, rNode));
	this->addLink(new Link(tNode, lNode));
	this->addLink(new Link(tNode, rNode));

	this->computeAabb();
	this->updateHingeScale();
}

void Graph::makeChair(double legL)
{
	this->clear();
	QVector<Vector3> xyz = XYZ();

	Box backBox(Point(0.25, 2, 0), xyz, Vector3(0.25, 2, 2));
	Node* backNode = new Node(backBox, "back");

	Box seatBox(Point(2, -0.5, 0), xyz, Vector3(2, 0.5, 2));
	Node* seatNode = new Node(seatBox, "seat_base");

	Box legBox0(Point(0.25, -legL-1, 1.75), xyz, Vector3(0.25, legL, 0.25));
	Node* legNode0 = new Node(legBox0, "leg0");

	Box legBox1(Point(0.25, -legL-1, -1.75), xyz, Vector3(0.25, legL, 0.25));
	Node* legNode1 = new Node(legBox1, "leg1");

	Box legBox2(Point(3.75, -legL-1, 1.75), xyz, Vector3(0.25, legL, 0.25));
	Node* legNode2 = new Node(legBox2, "leg2");

	Box legBox3(Point(3.75, -legL-1, -1.75), xyz, Vector3(0.25, legL, 0.25));
	Node* legNode3 = new Node(legBox3, "leg3");

	this->addNode(backNode);
	this->addNode(seatNode);
	this->addNode(legNode0);
	this->addNode(legNode1);
	this->addNode(legNode2);
	this->addNode(legNode3);

	this->addLink(new Link(backNode, seatNode));
	this->addLink(new Link(seatNode, legNode0));
	this->addLink(new Link(seatNode, legNode1));
	this->addLink(new Link(seatNode, legNode2));
	this->addLink(new Link(seatNode, legNode3));

	this->computeAabb();
	this->updateHingeScale();
}

void Graph::computeAabb()
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

double Graph::getAabbVolume()
{
	this->computeAabb();
	Vector3 diagonal = bbmax - bbmin;
	return diagonal[0] * diagonal[1] * diagonal[2];
}


void Graph::restoreConfiguration()
{
	if(isEmpty()) return;
	this->resetTags();

	QQueue<Node*> activeNodes;

	// starting from base node
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

bool Graph::isEmpty()
{
	return nodes.isEmpty();
}

void Graph::resetTags()
{
	foreach(Node* n, nodes) n->isFixed = false;
	foreach(Link* l, links) l->isFixed = false;
}

int Graph::nbNodes()
{
	return nodes.size();
}

int Graph::nbLinks()
{
	return links.size();
}

Node* Graph::getBaseNode()
{
	foreach(Node* n, nodes)
		if(n->mID.contains("_base"))
			return n;

	return NULL;
}

double Graph::getMtlVolume()
{
	double volume = 0;
	foreach (Node* n, nodes) volume += n->getVolume();
	return volume;
}

GraphState Graph::getState()
{
	GraphState state;

	foreach(Node* n, nodes) state.node_scale_factor.push_back(n->scaleFactor);
	foreach(Link* l, links)
	{
		state.active_hinge_id.push_back(l->getActiveHingeId());
		state.hinge_angle.push_back(l->activeHinge()->angle);
		state.link_is_broken.push_back(l->isBroken);
		state.link_is_nailed.push_back(l->isNailed);
	}

	return state;
}

void Graph::setState( GraphState &state )
{
	for (int i = 0; i < nodes.size(); i++)
	{
		nodes[i]->scaleFactor = state.node_scale_factor[i];
	}

	for (int i = 0; i < links.size(); i++)
	{
		links[i]->setActiveHingeId(state.active_hinge_id[i]);
		links[i]->activeHinge()->angle = state.hinge_angle[i];
		links[i]->isBroken = state.link_is_broken[i];
		links[i]->isNailed = state.link_is_nailed[i];
	}
}

QVector<Node*> Graph::getHotNodes(bool hot)
{
	QVector<Node*> hotNodes;
	foreach (Node* n, nodes)
		if (n->isHot == hot) hotNodes.push_back(n);

	return hotNodes;
}

QVector<Link*> Graph::getHotLinks(bool hot)
{
	QVector<Link*> hotLinks;
	foreach(Link* l, links)
		if (l->isHot == hot) hotLinks.push_back(l);

	return hotLinks;
}

Box Graph::getAabbBox()
{
	return Box(center, XYZ(), (bbmax-bbmin)/2);
}

void Graph::saveAsObj()
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



bool Graph::isHingable( Link* link )
{
	GraphState backupState = this->getState();

	

	this->setState(backupState);

	return true;
}

void Graph::updateHingeScale()
{
	foreach(Link* l, links)
		l->setHingeScale(this->radius / 10);
}

void Graph::hotAnalyze()
{
	// reset hot tags
	foreach(Node* n, nodes) n->isHot = false;
	foreach(Link* l, links) l->isHot = false;

	// get planes of the shrunk AABB
	this->computeAabb();
	Box aabbBox = getAabbBox();
	aabbBox.uniformScale(0.99);
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


void Graph::detectHinges( bool ee /*= true*/, bool ef /*= true*/, bool ff /*= true*/ )
{
	foreach (Link* l, links)
		l->detectHinges(ee, ef, ff);

	this->updateHingeScale();
}
