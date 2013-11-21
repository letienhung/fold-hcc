#include "HingeDetector.h"
#include "Segment.h"
#include "Box2.h"
#include "Numeric.h"

HingeDetector::HingeDetector(Node *n0, Node *n1)
{
	this->node0 = n0;
	this->node1 = n1;

	// tag: whether a edge is already a hinge
	this->node0->mBox.edgeTags = QVector<bool>(Box::NB_EDGES, false);
	this->node1->mBox.edgeTags = QVector<bool>(Box::NB_EDGES, false);
}


QVector<Hinge*> HingeDetector::getHinges()
{
	QVector<Hinge*> hinges;

	hinges += getEdgeEdgeHinges(node0, node1);
	hinges += getEdgeEdgeHinges(node1, node0);
	hinges += getEdgeFaceHinges(node0, node1);
	hinges += getEdgeFaceHinges(node1, node0);

	return hinges;
}

void HingeDetector::getPerpAxisAndExtent( Box& box, Vector3 hinge_center, Vector3 hinge_axis, QVector<Vector3>& perpAxis, QVector<double> &perpExtent )
{
	perpAxis.clear();
	perpExtent.clear();

	Vec3d h2n = (box.Center - hinge_center).normalized();

	for(int i = 0; i < 3; i++){
		if (isPerp(box.Axis[i], hinge_axis))
		{
			Vector3 dd = box.Axis[i];
			if (dot(dd, h2n) < 0) dd *= -1;

			perpAxis.push_back(dd);
			perpExtent.push_back(box.Extent[i]);
		}
	}
}

QVector<Hinge*> HingeDetector::getEdgeEdgeHinges(Node* n0, Node* n1)
{
	QVector<Hinge*> hinges;

	Box&				box0 = n0->mBox;
	Box&				box1 = n1->mBox;
	QVector<Segment>	edges0 = box0.getEdgeSegments();
	QVector<Segment>	edges1 = box1.getEdgeSegments();

	for (int i = 0; i < 12; i++)
	{
		// skip if e0_i is already a hinge
		if (box0.edgeTags[i]) continue;

		for (int j = 0; j < 12; j++)
		{
			Segment& e0_i = edges0[i];
			Segment& e1_j = edges1[j];

			// create a hinge when one edge contains the other
			if (e1_j.contains(e0_i))
			{
				// generate hinge for e1_j
				Hinge* h = this->generateEdgeEdgeHinge(n0, n1, e0_i, e1_j);
				if (h)
				{
					hinges.push_back(h);
					box0.edgeTags[i] = true;

					// if two edges are identical, mark both
					// therefor two overlapping edges will be detected only once
					if (e0_i.contains(e1_j))
						box1.edgeTags[j] = true;

					break;
				}
			}
		}
	}

	return hinges;
}

QVector<Hinge*> HingeDetector::getEdgeFaceHinges(Node* n0, Node* n1)
{
	QVector<Hinge*> hinges;

	Box&				box0 = n0->mBox;
	Box&				box1 = n1->mBox;
	QVector<Segment>	edges0 = box0.getEdgeSegments();
	QVector<Box2>		faces1 = box1.getFaceRectangles();

	// Edge0-Face1 test
	for (int i = 0; i < 12; i ++){
		// skip if e_i is already a hinge
		if (box0.edgeTags[i]) continue;

		for (int j = 0; j < 6; j++)
		{
			Segment& e_i = edges0[i];
			Box2& f_j = faces1[j];

			// create a hinge if f1_j contains e0_i
			if (f_j.contains(e_i))
			{
				Hinge* h = generateEdgeFaceHinge(n0, n1, e_i, f_j);
				if (h) 
				{
					hinges.push_back(h);
					// one edge generate at most one hinge
					// if an edge is on two faces, skip the second
					break;
				}
			}
		}
	}

	return hinges;
}

// e0 \in f1 : create hinge for e0
Hinge* HingeDetector::generateEdgeFaceHinge( Node* n0, Node* n1, Segment& e0, Box2& f1 )
{
	// center and hz
	Vector3 hcenter = e0.Center;
	Vector3 hz = e0.Direction;

	// hx has larger extent
	QVector<Vector3>	perpAxis0;
	QVector<double>		perpExtent0;
	getPerpAxisAndExtent(n0->mBox, hcenter, hz, perpAxis0, perpExtent0);
	if (perpAxis0.size() != 2)	return NULL;
	Vector3 hx = perpAxis0[0], hx_perp = perpAxis0[1];
	if (perpExtent0[0] < perpExtent0[1]) std::swap(hx, hx_perp);

	// hy is on f_j
	// HxCrossHxPerp and HxCrossHy have opposite directions 
	Vector3 hy = cross(f1.Normal, hz);
	Vector3 HxCrossHxPerp = cross(hx, hx_perp);
	Vector3 HxCrossHy = cross(hx, hy);
	if (dot(HxCrossHxPerp, HxCrossHy) > 0) hy *= -1;

	// update hz if need
	// HxCrossHxPerp and hz have opposite directions
	if (dot(HxCrossHxPerp, hz) > 0) hz *= -1;

	return new Hinge(node0, node1, hcenter, hx, hy, hz, M_PI/2);
}

// e0 \in e1 : create hinge for e0
Hinge* HingeDetector::generateEdgeEdgeHinge( Node* n0, Node* n1, Segment& e0, Segment& e1 )
{
	// center and hz
	Vector3 hcenter = e0.Center;
	Vector3 hz = e0.Direction;

	// data used to determine hx and hy
	QVector<Vector3>	perpAxis0, perpAxis1;
	QVector<double>		perpExtent0, perpExtent1;
	getPerpAxisAndExtent(n0->mBox, hcenter, hz, perpAxis0, perpExtent0);
	getPerpAxisAndExtent(n1->mBox, hcenter, hz, perpAxis1, perpExtent1);
	if (perpAxis0.size() != 2 || perpAxis1.size() != 2)	return NULL;

	// hx has larger extent
	Vector3 hx = perpAxis0[0], hx_perp = perpAxis0[1];
	if (perpExtent0[0] < perpExtent0[1]) std::swap(hx, hx_perp);

	// hy
	// HxCrossHxPerp and HyCrossHyPerp have opposite directions 
	Vector3 hy = perpAxis1[0], hy_perp = perpAxis1[1];
	Vector3 HxCrossHxPerp = cross(hx, hx_perp);
	Vector3 HyCrossHyPerp = cross(hy, hy_perp);
	if (dot(HxCrossHxPerp, HyCrossHyPerp) > 0) hy = hy_perp;

	// update hz if need
	// HxCrossHxPerp and hz have opposite directions
	if (dot(HxCrossHxPerp, hz) > 0) hz *= -1;

	return new Hinge(node0, node1, hcenter, hx, hy, hz, M_PI);
}
