#include "Link.h"
#include "Node.h"

Link::Link()
{
	angle = 0.0;

    node1 = NULL;
    node2 = NULL;
}

Link::Link(Node* n1, Node* n2)
{
    node1 = n1;
    node2 = n2;
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

void Link::draw()
{
}
