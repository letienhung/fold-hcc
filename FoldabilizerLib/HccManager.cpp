#include "HccManager.h"
#include "Numeric.h"

HccManager::HccManager()
{
	hccGraph = new HccGraph();
}

HccManager::~HccManager()
{
	if (hccGraph) delete hccGraph;
}

void HccManager::draw()
{
	if (hccGraph) hccGraph->draw();
}

void HccManager::makeI()
{
	hccGraph->clear();
	QVector<Vector3> xyz = XYZ();

	double thk = 0.5;
	Node* tNode = new Node(Box(Point(thk, 6, 0), xyz, Vector3(thk, 2, thk)), "top");
	Node* bNode = new Node(Box(Point(thk, 2, 0), xyz, Vector3(thk, 2, thk)), "bottom");

	hccGraph->addNode(tNode);
	hccGraph->addNode(bNode);
	hccGraph->addLink(new Link(tNode, bNode));

	hccGraph->computeAabb();
	hccGraph->updateHingeScale();
	emit(activeHccChanged());
}

void HccManager::makeL()
{
	hccGraph->clear();
	QVector<Vector3> xyz = XYZ();

	Node* vNode = new Node(Box(Point(-0.5, 2, 0), xyz, Vector3(0.5, 2, 2)), "vBox");
	Node* hNode = new Node(Box(Point(2, -0.5, 0), xyz, Vector3(2, 0.5, 2)), "hBox");

	hccGraph->addNode(vNode); hccGraph->addNode(hNode);
	hccGraph->addLink(new Link(vNode, hNode));

	hccGraph->computeAabb();
	hccGraph->updateHingeScale();
	emit(activeHccChanged());
}

void HccManager::makeT()
{
	hccGraph->clear();
	QVector<Vector3> xyz = XYZ();

	Node* vNode = new Node(Box(Point(0, -2, 0), xyz, Vector3(0.5, 2, 2)), "vBox");

	QVector<Vector3> rot_xyz;
	rot_xyz << Vector3(1, 0, 1) << Vector3(0, 1, 0) << Vector3(-1, 0, 1);
	Node* hNode = new Node(Box(Point(0, 0.5, 0), rot_xyz, Vector3(2, 0.5, 2)), "hBox_base");

	hccGraph->addNode(vNode);	hccGraph->addNode(hNode);
	hccGraph->addLink(new Link(vNode, hNode));

	hccGraph->computeAabb();
	hccGraph->updateHingeScale();
	emit(activeHccChanged());
}

void HccManager::makeX()
{
	hccGraph->clear();
	QVector<Vector3> xyz = XYZ();

	Node* vNode = new Node(Box(Point(0, 0, 0.5), xyz, Vector3(0.5, 4, 0.5)), "vBox");
	Node* hNode = new Node(Box(Point(0, 0, -0.5), xyz, Vector3(4, 0.5, 0.5)), "hBox");

	hccGraph->addNode(vNode);	hccGraph->addNode(hNode);
	hccGraph->addLink(new Link(vNode, hNode));

	hccGraph->computeAabb();
	hccGraph->updateHingeScale();
	emit(activeHccChanged());
}

void HccManager::makeSharp()
{
	hccGraph->clear();
	QVector<Vector3> xyz = XYZ();

	Node* vlNode = new Node(Box(Point(-2,  0,  0.5), xyz, Vector3(0.5, 4, 0.5)), "v-left_base"); 
	Node* vrNode = new Node(Box(Point( 2,  0,  0.5), xyz, Vector3(0.5, 4, 0.5)), "v-right");
	Node* htNode = new Node(Box(Point( 0,  2, -0.5), xyz, Vector3(4, 0.5, 0.5)), "h-top");
	Node* hbNode = new Node(Box(Point( 0, -2, -0.5), xyz, Vector3(4, 0.5, 0.5)), "h-bottom");

	hccGraph->addNode(vlNode); hccGraph->addNode(vrNode); hccGraph->addNode(htNode); hccGraph->addNode(hbNode);

	hccGraph->addLink(new Link(vlNode, hbNode));
	hccGraph->addLink(new Link(vrNode, hbNode));
	hccGraph->addLink(new Link(vlNode, htNode));
	hccGraph->addLink(new Link(vrNode, htNode));

	hccGraph->computeAabb();
	hccGraph->updateHingeScale();
	emit(activeHccChanged());
}

void HccManager::makeU( double uleft, double umid, double uright )
{
	hccGraph->clear();
	QVector<Vector3> xyz = XYZ();

	Node* ltNode = new Node(Box(Point(0.5, 3 * uleft, 0), xyz, Vector3(0.5, uleft, 0.5)), "left-top");
	Node* lbNode = new Node(Box(Point(0.5, uleft, 0), xyz, Vector3(0.5, uleft, 0.5)), "left-bottom");
	Node* hNode = new Node(Box(Point(umid, -0.5, 0), xyz, Vector3(umid, 0.5, 0.5)), "horizontal_base");
	Node* rNode = new Node(Box(Point(2 * umid - 0.5, uright, 0), xyz, Vector3(0.5, uright, 0.5)), "right");

	hccGraph->addNode(lbNode);hccGraph->addNode(ltNode);hccGraph->addNode(hNode);hccGraph->addNode(rNode);
	hccGraph->addLink(new Link(ltNode, lbNode));	
	hccGraph->addLink(new Link(lbNode, hNode));

	Link* nailedLink = new Link(rNode, hNode);
	//nailedLink->isNailed = true;
	this->hccGraph->addLink(nailedLink);

	hccGraph->computeAabb();
	hccGraph->updateHingeScale();
	emit(activeHccChanged());
}

void HccManager::makeO()
{
	hccGraph->clear();
	QVector<Vector3> xyz = XYZ();

	Node* lNode = new Node(Box(Point(0.5, 2, 0), xyz, Vector3(0.5, 2, 0.5)), "left");
	Node* rNode = new Node(Box(Point(7.5, 2, 0), xyz, Vector3(0.5, 2, 0.5)), "right");      
	Node* bNode = new Node(Box(Point(4, -0.5, 0), xyz, Vector3(3, 0.5, 0.5)), "bottom_base");
	Node* tNode = new Node(Box(Point(4, 4.5, 0), xyz, Vector3(3, 0.5, 0.5)), "top");

	hccGraph->addNode(lNode);hccGraph->addNode(rNode);hccGraph->addNode(bNode);hccGraph->addNode(tNode);
	hccGraph->addLink(new Link(bNode, lNode));
	hccGraph->addLink(new Link(bNode, rNode));
	hccGraph->addLink(new Link(tNode, lNode));
	hccGraph->addLink(new Link(tNode, rNode));

	hccGraph->computeAabb();
	hccGraph->updateHingeScale();
	emit(activeHccChanged());
}

void HccManager::makeO_2()
{
	hccGraph->clear();
	QVector<Vector3> xyz = XYZ();

	Node* lNode = new Node(Box(Point(0.5, 2, 0), xyz, Vector3(0.5, 2, 0.5)), "left");
	Node* rNode = new Node(Box(Point(9.5, 2, 0), xyz, Vector3(0.5, 2, 0.5)), "right");	
	Node* bNode = new Node(Box(Point(5, 0.5, 1), xyz, Vector3(5, 0.5, 0.5)), "bottom_base");
	Node* tNode = new Node(Box(Point(5, 3.5, 1), xyz, Vector3(5, 0.5, 0.5)), "top");

	hccGraph->addNode(lNode);hccGraph->addNode(rNode);hccGraph->addNode(bNode);hccGraph->addNode(tNode);
	hccGraph->addLink(new Link(bNode, lNode));
	hccGraph->addLink(new Link(bNode, rNode));
	hccGraph->addLink(new Link(tNode, lNode));
	hccGraph->addLink(new Link(tNode, rNode));

	hccGraph->computeAabb();
	hccGraph->updateHingeScale();
	emit(activeHccChanged());
}

void HccManager::makeBox()
{
	activeHcc()->clear();
	QVector<Vector3> xyz = XYZ();

	double thk = 0.2;
	Node* back = new Node(Box(Point(-thk, 2, 0), xyz, Vector3(thk, 2, 2)), "back");
	Node* bottom = new Node(Box(Point(2, -thk, 0), xyz, Vector3(2, thk, 2)), "bottom_base");
	Node* left = new Node(Box(Point(2, 1, -2+thk), xyz, Vector3(1, 1, thk)), "left");
	Node* right = new Node(Box(Point(2, 1, 2-thk), xyz, Vector3(1, 1, thk)), "right");
	Node* front = new Node(Box(Point(4-thk, 1, 0), xyz, Vector3(thk, 1, 1)), "front");

	hccGraph->addNode(bottom); 
	hccGraph->addNode(back); 
	hccGraph->addNode(front);
	hccGraph->addNode(left); 
	hccGraph->addNode(right); 

	hccGraph->addLink(bottom, back); 
	hccGraph->addLink(bottom, front);
	hccGraph->addLink(bottom, left); hccGraph->addLink(bottom, right); 

	hccGraph->computeAabb();
	hccGraph->updateHingeScale();
	emit(activeHccChanged());
}

void HccManager::makeChair(double legL)
{
	activeHcc()->clear();
	QVector<Vector3> xyz = XYZ();

	Node* backNode = new Node(Box(Point(0.25, 2, 0), xyz, Vector3(0.25, 2, 2)), "back");
	Node* seatNode = new Node(Box(Point(2, -0.5, 0), xyz, Vector3(2, 0.5, 2)), "seat_base");
	Node* legNode0 = new Node(Box(Point(0.25, -legL-1, 1.75), xyz, Vector3(0.25, legL, 0.25)), "leg0");
	Node* legNode1 = new Node(Box(Point(0.25, -legL-1, -1.75), xyz, Vector3(0.25, legL, 0.25)), "leg1");
	Node* legNode2 = new Node(Box(Point(3.75, -legL-1, 1.75), xyz, Vector3(0.25, legL, 0.25)), "leg2");
	Node* legNode3 = new Node(Box(Point(3.75, -legL-1, -1.75), xyz, Vector3(0.25, legL, 0.25)), "leg3");

	hccGraph->addNode(backNode);
	hccGraph->addNode(seatNode);
	hccGraph->addNode(legNode0);
	hccGraph->addNode(legNode1);
	hccGraph->addNode(legNode2);
	hccGraph->addNode(legNode3);
	hccGraph->addLink(new Link(backNode, seatNode));
	hccGraph->addLink(new Link(seatNode, legNode0));
	hccGraph->addLink(new Link(seatNode, legNode1));
	hccGraph->addLink(new Link(seatNode, legNode2));
	hccGraph->addLink(new Link(seatNode, legNode3));

	hccGraph->computeAabb();
	hccGraph->updateHingeScale();
	emit(activeHccChanged());
}

HccGraph* HccManager::activeHcc()
{
	return hccGraph;
}

void HccManager::loadHCC( QString filename )
{
	activeHcc()->loadHCC(filename);
	emit(activeHccChanged());
}
