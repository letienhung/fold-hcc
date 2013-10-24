#include "Link.h"
#include "Node.h"

Link::Link()
{
	from = Point(0,0,0);
	to = from;

	angle = 0.0;
	score = 0.0;

	isFolded = false;

    node1 = NULL;
    node2 = NULL;
}

Link::Link(Point &f, Point &t, Node* n1, Node* n2)
{
	from = f;
	to = t;

	isFolded = false;
	
    node1 = n1;
    node2 = n2;

	angle = calAngle();
	score = calScore();
}

void Link::setNode(Node* n1, Node* n2)
{
    node1 = n1;
    node2 = n2;
}

Node* Link::getNode1()
{
    return node1;
}

Node* Link::getNode2()
{
    return node2;
}

double Link::calAngle()
{
	//TODO
	return 0.0;
}

double Link::calScore()
{
	//TODO
	return 0.0;
}

void Link::draw()
{
	//TODO
}
