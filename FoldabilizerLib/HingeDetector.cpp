#include "HingeDetector.h"
#include "Segment.h"
#include "Rectangle.h"
#include "IntrRectRect.h"
#include "Numeric.h"

using namespace Geom;

HingeDetector::HingeDetector(Node *n0, Node *n1)
{
	this->node0 = n0;
	this->node1 = n1;

	// tag: whether a edge is already a hinge
	this->node0->mBox.edgeTags = QVector<bool>(Box::NB_EDGES, false);
	this->node1->mBox.edgeTags = QVector<bool>(Box::NB_EDGES, false);
}

QVector<Hinge*> HingeDetector::getHinges(bool ee, bool ef, bool ff)
{
	QVector<Hinge*> hinges;
	if (ee)
	{
		hinges += getEdgeEdgeHinges(node0, node1);
		hinges += getEdgeEdgeHinges(node1, node0);
	}
	else
	{	// the detection of EF depends on EE
		this->getEdgeEdgeHinges(node0, node1);
		this->getEdgeEdgeHinges(node1, node0);
	}


	if (ef)
	{
		hinges += getEdgeFaceHinges(node0, node1);
		hinges += getEdgeFaceHinges(node1, node0);
	}

	if (ff)
		hinges += getFaceFaceHinges();

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
	QVector<Geom::Rectangle> faces1 = box1.getFaceRectangles();

	// Edge0-Face1 test
	for (int i = 0; i < 12; i ++){
		// skip if e_i is already a hinge
		if (box0.edgeTags[i]) continue;

		for (int j = 0; j < 6; j++)
		{
			Segment& e_i = edges0[i];
			Geom::Rectangle& f_j = faces1[j];

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

QVector<Hinge*> HingeDetector::getFaceFaceHinges()
{
	QVector<Hinge*> hinges;

	Box&				box0 = node0->mBox;
	Box&				box1 = node1->mBox;
	QVector<Geom::Rectangle>		faces0 = box0.getFaceRectangles();
	QVector<Geom::Rectangle>		faces1 = box1.getFaceRectangles();

	for (int i = 0; i < Box::NB_FACES; i++)
	{
		for (int j = 0; j < Box::NB_FACES; j++)
		{
			Geom::Rectangle &f0 = faces0[i], &f1 = faces1[j];

			// skip if two faces are not coplanar
			if (!f0.isCoplanarWith(f1)) continue;

			// skip if one contains the other
			if (f0.contains(f1) || f1.contains(f0)) continue;

			// compute intersection
			IntrRectRect intersector;
			QVector<Vector3> intrPnts = intersector.test(f0, f1);

			// create hinge is f0 and f1 intersect
			if (intrPnts.size() >= 3)
			{
				// hcenter
				Vector3 hcenter(0,0,0);
				foreach(Vector3 p, intrPnts) hcenter += p;
				hcenter /= intrPnts.size();

				// hz
				Vector3 hz = f0.Normal;

				// data used to determine hx and hy
				QVector<Vector3>	perpAxis0, perpAxis1;
				QVector<double>		perpExtent0, perpExtent1;
				getPerpAxisAndExtent(node0->mBox, hcenter, hz, perpAxis0, perpExtent0);
				getPerpAxisAndExtent(node1->mBox, hcenter, hz, perpAxis1, perpExtent1);
				if (perpAxis0.size() != 2 || perpAxis1.size() != 2)	continue;

				// hx is the axis with larger extent in n0
				// so is hy
				Vector3 hx = (perpExtent0[0] >= perpExtent0[1]) ? perpAxis0[0] : perpAxis0[1];
				Vector3 hy = (perpExtent1[0] >= perpExtent1[1]) ? perpAxis1[0] : perpAxis1[1];

				hinges.push_back(new Hinge(node0, node1, hcenter, hx, hy, hz, 2 * M_PI));

				// debug
				//foreach(Vector3 v, intrPnts) node0->debug_points.push_back(v);
			}
		}
	}

	return hinges;
}


// e0 \in f1 : create hinge for e0
Hinge* HingeDetector::generateEdgeFaceHinge( Node* n0, Node* n1, Segment& e0, Geom::Rectangle& f1 )
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

	return new Hinge(n0, n1, hcenter, hx, hy, hz, M_PI_2);
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

	return new Hinge(n0, n1, hcenter, hx, hy, hz, M_PI);
}