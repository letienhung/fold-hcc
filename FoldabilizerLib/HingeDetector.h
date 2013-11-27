#pragma once

#include "Node.h"
#include "Hinge.h"

class HingeDetector
{
private:
	Node *node0, *node1;

public:
    HingeDetector( Node *n0, Node *n1 );

	QVector<Hinge*> getHinges(bool ee = true, bool ef = true, bool ff = true);
	QVector<Hinge*> getEdgeEdgeHinges(Node* n0, Node* n1);
	QVector<Hinge*> getEdgeFaceHinges(Node* n0, Node* n1);
	QVector<Hinge*> getFaceFaceHinges();

	void	getPerpAxisAndExtent( Geom::Box& box, Vector3 hinge_center, Vector3 hinge_axis, QVector<Vector3>& perpAxis, QVector<double> &perpExtent );
	Hinge*	generateEdgeEdgeHinge( Node* n0, Node* n1, Geom::Segment& e0, Geom::Segment& e1 );
	Hinge*	generateEdgeFaceHinge( Node* n0, Node* n1, Geom::Segment& e0, Geom::Rectangle& f1 );
};
