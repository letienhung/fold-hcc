#pragma once

#include "Node.h"
#include "Hinge.h"

using namespace Goem;

class HingeDetector
{
private:
	Node *node0, *node1;

public:
    HingeDetector( Node *n0, Node *n1 );

	QVector<Hinge*> getHinges();
	QVector<Hinge*> getEdgeEdgeHinges(Node* n0, Node* n1);
	QVector<Hinge*> getEdgeFaceHinges(Node* n0, Node* n1);
	QVector<Hinge*> getFaceFaceHinges();

	void	getPerpAxisAndExtent( Box& box, Vector3 hinge_center, Vector3 hinge_axis, QVector<Vector3>& perpAxis, QVector<double> &perpExtent );
	Hinge*	generateEdgeEdgeHinge( Node* n0, Node* n1, Segment& e0, Segment& e1 );
	Hinge*	generateEdgeFaceHinge( Node* n0, Node* n1, Segment& e0, Goem::Rectangle& f1 );
};
