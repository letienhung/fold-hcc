#include "Graph.h"

Graph::Graph()
{
	makeL();
}

Graph::Graph(QString &fname)
{
	if(!parseHCC(fname))
		qDebug()<<"Failed to load HCC file!\n";
}

Graph::~Graph()
{
    clearList();
}

void Graph::clearList()
{
    int nSize = nodes.size();
	if(nSize){
		for (int i = 0; i < nSize; i++)
			delete nodes[i];
		nodes.clear();
        links.clear();
	}
}

void Graph::addNode(Node* node)
{
    nodes.push_back(node);
}

bool Graph::parseHCC(QString &fname)
{
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
			Point center = Point(node_2.toElement().attribute("centerX").toDouble(),
				                 node_2.toElement().attribute("centerY").toDouble(),
								 node_2.toElement().attribute("centerZ").toDouble());

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
			int id = node_2.toElement().attribute("ID").toInt();

			QString box1ID = node_2.toElement().attribute("Box1ID");
			QString box2ID = node_2.toElement().attribute("Box2ID");

			Link* link = new Link();
			Node *n1 = getNode(box1ID);
			Node *n2 = getNode(box2ID);
			link->setNode(n1, n2);

			n1->linkList.push_back(link);
			n2->linkList.push_back(link);
			links.push_back(link);

			node_2 = node_2.nextSibling();
		}
	}

	return true;
}

QVector<Node *> Graph::getAdjnode(Node* node)
{
    return node->getAdjnodes();
}

QVector<Node *> Graph::getLeafnode()
{
	QVector<Node *> leafnodes;
	
    int nSize = nodes.size();
	for(int i = 0; i < nSize; i++)
        if (nodes[i]->linkList.size() == 1)
            leafnodes.push_back(nodes[i]);

   return leafnodes;
}

Node* Graph::getNode(QString& id)
{
	foreach(Node* n, nodes)
		if(n->mID == id)
			return n;
}

void Graph::draw()
{
	foreach(Node *n, nodes) n->draw();
	foreach(Link *l, links) l->draw();
}


void Graph::makeL()
{
	QVector<Vector3> xyz;
	xyz.push_back(Vector3(1, 0, 0));
	xyz.push_back(Vector3(0, 1, 0));
	xyz.push_back(Vector3(0, 0, 1));

	Box vBox(Point(-0.5, 2, 0), xyz, Vector3(0.5, 2, 2));
	Box hBox(Point(2, -0.5, 0), xyz, Vector3(2, 0.5, 2));

	Node* vNode = new Node(vBox, "vBox");
	Node* hNode = new Node(hBox, "hBox");

	this->addNode(vNode);
	this->addNode(hNode);
}