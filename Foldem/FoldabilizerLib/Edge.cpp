#include "Edge.h"

Edge::Edge()
{
	from = Point(0,0,0);
	to = from;

	angle = 0.0;
	score = 0.0;

	isFolded = false;

    node1 = NULL;
    node2 = NULL;
}

Edge::Edge(Point &mfrom, Point &mto, Node* mNode1, Node* mNode2)
{
	from = mfrom;
	to = mto;

	isFolded = false;
	
    node1 = mNode1;
    node2 = mNode2;

	angle = calAngle();
	score = calScore();
}

void Edge::setCuboid(Node* mNode1, Node* mNode2)
{
    node1 = mNode1;
    node2 = mNode2;
}

Node* Edge::getNode1()
{
    return node1;
}

Node* Edge::getNode2()
{
    return node2;
}

double Edge::calAngle()
{
	//TODO
	return 0.0;
}

double Edge::calScore()
{
	//TODO
	return 0.0;
}

void Edge::draw()
{
	//TODO
}
