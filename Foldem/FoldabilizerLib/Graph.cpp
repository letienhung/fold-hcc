#include "Graph.h"
#include "xmlWriter.h"

Graph::Graph()
{
}

Graph::Graph(QString fname)
{
	if(!parseHCC(fname))
		qDebug()<<"Failed to load HCC file!\n";
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

bool Graph::parseHCC(QString fname)
{
	this->clear();

	QFile file(fname);
	file.open(QIODevice::ReadOnly);

	if(!file.isOpen()) return false;

	QDomDocument doc;
	if (!doc.setContent(&file)) return false;

	QDomElement root = doc.documentElement();
	if(root.tagName() != "Graph")
		return false;

	QDomNode node;
	QDomNode node_2;

	node = root.firstChild();
	//number of boxes
	int nBox = node.toElement().attribute("BoxAmount").toInt();
	int nLink = node.toElement().attribute("LinkAmount").toInt();

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

			//Might add position 

			node_2 = node_2.nextSibling();
		}
	}

	node = node.nextSibling();
	{
		node_2 = node.toElement().firstChild();

		while(!node_2.isNull())
		{
			//int id = node_2.toElement().attribute("ID").toInt();

			QString box1ID = node_2.toElement().attribute("Box1ID");
			QString box2ID = node_2.toElement().attribute("Box2ID");

			Vector3 from = Vector3(node_2.toElement().attribute("CenterX").toDouble(),
			node_2.toElement().attribute("CenterY").toDouble(),
			node_2.toElement().attribute("CenterZ").toDouble());

			Vector3 to = Vector3(node_2.toElement().attribute("ToX").toDouble(),
			node_2.toElement().attribute("ToY").toDouble(),
			node_2.toElement().attribute("ToZ").toDouble());
			
			/*Node *n1 = getNode(box1ID);
			Node *n2 = getNode(box2ID);
			Link* link = new Link();
			link->setNode(n1, n2);

			this->addLink(box1ID, box2ID);*/

			node_2 = node_2.nextSibling();
		}
	}

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
		link_attrs.insert("CenterX", QString::number(links[i]->center.x()));
		link_attrs.insert("CenterY ", QString::number(links[i]->center.y()));
		link_attrs.insert("CenterZ", QString::number(links[i]->center.z()));
		link_attrs.insert("ToX ", QString::number(links[i]->center.x()+links[i]->axis.x()));
		link_attrs.insert("ToY ", QString::number(links[i]->center.y()+links[i]->axis.y()));
		link_attrs.insert("ToZ", QString::number(links[i]->center.z()+links[i]->axis.z()));
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
	this->addLink(new Link(tNode, bNode, Vector3(1,4,0), Vector3(0,0,1)));
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

	this->addLink(new Link(vNode, hNode, Vector3(0,0,0), Vector3(0,0,1)));
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
	Node* hNode = new Node(hBox, "hBox");

	this->addNode(vNode);
	this->addNode(hNode);
	this->addLink(new Link(vNode, hNode, Vector3(0.5,0,0), Vector3(0,0,1)));
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

	this->addLink(new Link(vNode, hNode, Vector3(0,0,0), Vector3(0,0,1)));
}

void Graph::makeU()
{
	this->clear();
	QVector<Vector3> xyz = XYZ();

	Box ltBox(Point(0.5, 6, 0), xyz, Vector3(0.5, 2, 0.5));
	Node* ltNode = new Node(ltBox, "left-top");

	Box lbBox(Point(0.5, 2, 0), xyz, Vector3(0.5, 2, 0.5));
	Node* lbNode = new Node(lbBox, "left-bottom");

	Box hBox(Point(4, -0.5, 0), xyz, Vector3(4, 0.5, 0.5));
	Node* hNode = new Node(hBox, "horizontal");

	Box rBox(Point(7.5, 2, 0), xyz, Vector3(0.5, 2, 0.5));
	Node* rNode = new Node(rBox, "right");

	this->addNode(rNode);
	this->addNode(lbNode);
	this->addNode(hNode);
	this->addNode(ltNode);

	this->addLink(new Link(ltNode, lbNode, Vector3(0,4,0), Vector3(0,0,1)));
	this->addLink(new Link(lbNode, hNode, Vector3(1,0,0), Vector3(0,0,1)));
	Link* nailedLink = new Link(hNode, rNode, Vector3(7,0,0), Vector3(0,0,1));
	//nailedLink->isNailed = true;
	this->addLink(nailedLink);

	nodes[0]->scaleFactor = 0.9;
	nodes[0]->scaleFactor = 1.2;
	nodes[0]->scaleFactor = 0.6;
	nodes[0]->scaleFactor = 1.8;
}

void Graph::makeChair()
{
	this->clear();
	QVector<Vector3> xyz = XYZ();

	Box backBox(Point(0.25, 2, 0), xyz, Vector3(0.25, 2, 2));
	Node* backNode = new Node(backBox, "back");

	Box seatBox(Point(2, -0.5, 0), xyz, Vector3(2, 0.5, 2));
	Node* seatNode = new Node(seatBox, "seat");

	Box legBox0(Point(0.25, -3, 1.75), xyz, Vector3(0.25, 2, 0.25));
	Node* legNode0 = new Node(legBox0, "leg0");

	Box legBox1(Point(0.25, -3, -1.75), xyz, Vector3(0.25, 2, 0.25));
	Node* legNode1 = new Node(legBox1, "leg1");

	Box legBox2(Point(3.75, -3, 1.75), xyz, Vector3(0.25, 2, 0.25));
	Node* legNode2 = new Node(legBox2, "leg2");

	Box legBox3(Point(3.75, -3, -1.75), xyz, Vector3(0.25, 2, 0.25));
	Node* legNode3 = new Node(legBox3, "leg3");

	this->addNode(backNode);
	this->addNode(seatNode);
	this->addNode(legNode0);
	this->addNode(legNode1);
	this->addNode(legNode2);
	this->addNode(legNode3);
}

void Graph::computeAABB()
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
			QVector<Point> conners = n->getBoxConners();
			foreach(Point p, conners) {
				bbmin = minimize(bbmin, p);
				bbmax = maximize(bbmax, p);
			}		
		}

		center = (bbmin + bbmax) * 0.5f;
		radius = (bbmax - bbmin).norm() * 0.5f;
	}
}

void Graph::jump()
{
	foreach(Node* n, nodes) n->changeScaleFactor();
	foreach(Link* l, links) l->changeAngle();
}


void Graph::restoreConfiguration()
{
	if(isEmpty()) return;
	this->resetTags();

	QQueue<Node*> activeNodes;

	// starting from node[0]
	nodes[0]->fix();
	nodes[0]->isFixed = true;
	activeNodes.enqueue(nodes[0]);

	while (!activeNodes.isEmpty())
	{
		Node* anode = activeNodes.dequeue();
		foreach(Link* l, getLinks(anode->mID))
		{
			if (l->fix()) activeNodes.enqueue(l->otherNode(anode->mID));
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
