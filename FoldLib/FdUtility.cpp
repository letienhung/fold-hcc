#include "FdUtility.h"

#include "RodNode.h"
#include "PatchNode.h"

#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "DistRectRect.h"
#include "Numeric.h"
#include "MinOBB.h"
#include "PcaOBB.h"

FdNodeArray2D clusterNodes( QVector<FdNode*> nodes, double disThr )
{
	FdNodeArray2D clusters;
	if (nodes.isEmpty()) return clusters;

	// tags used for searching
	QVector<bool> visited(nodes.size(), false);
	int nbVisited = 0;

	// initial current graph
	QVector<FdNode*> curr_g;
	curr_g.push_back(nodes[0]);
	visited[0] = true;
	nbVisited++;

	// search for connected node to current graph
	while(nbVisited < nodes.size())
	{
		int preN = nbVisited;
		for (int i = 0; i < nodes.size(); i++)
		{
			// skip visited node
			if (visited[i]) continue;

			if (getDistance(nodes[i], curr_g) < disThr)
			{
				curr_g.push_back(nodes[i]);
				visited[i] = true;
				nbVisited++;
			}
		}

		// cannot find more nodes
		if (preN == nbVisited)
		{
			// save current graph
			clusters.push_back(curr_g);

			// reset current graph with an unvisited node
			for (int i = 0; i < nodes.size(); i++)
			{
				if (!visited[i])
				{
					curr_g.clear();
					curr_g.push_back(nodes[i]);
					visited[i] = true;
					nbVisited++;
					break;
				}
			}
		}
	}

	// save current graph
	clusters.push_back(curr_g);

	return clusters;
}

Geom::Segment getDistSegment( FdNode* n1, FdNode* n2 )
{
	Geom::Segment ds;

	if (n1->mType == FdNode::ROD)
	{
		RodNode* node1 = (RodNode*)n1;
		Geom::Segment rod1 = node1->mRod;

		// rod-rod
		if (n2->mType == FdNode::ROD)
		{
			RodNode* node2 = (RodNode*)n2;
			Geom::Segment rod2 = node2->mRod;

			Geom::DistSegSeg dss(rod1, rod2);
			ds.set(dss.mClosestPoint0, dss.mClosestPoint1);
		}
		// rod-patch
		else
		{
			PatchNode* node2 = (PatchNode*)n2;
			Geom::Rectangle rect2 = node2->mPatch;

			Geom::DistSegRect dsr(rod1, rect2);
			ds.set(dsr.mClosestPoint0, dsr.mClosestPoint1);
		}
	}

	else 
	{
		PatchNode* node1 = (PatchNode*)n1;
		Geom::Rectangle rect1 = node1->mPatch;

		// patch-rod
		if (n2->mType == FdNode::ROD)
		{
			RodNode* node2 = (RodNode*)n2;
			Geom::Segment rod2 = node2->mRod;

			Geom::DistSegRect dsr(rod2, rect1);
			ds.set(dsr.mClosestPoint0, dsr.mClosestPoint1);
		}
		// patch-patch
		else
		{
			PatchNode* node2 = (PatchNode*)n2;
			Geom::Rectangle rect2 = node2->mPatch;

			Geom::DistRectRect drr(rect1, rect2);
			ds.set(drr.mClosestPoint0, drr.mClosestPoint1);
		}	
	}

	return ds;
}

double getDistance( FdNode* n1, FdNode* n2 )
{
	return getDistSegment(n1, n2).length();
}

double getDistance( FdNode* n, QVector<FdNode*> nset )
{
	double minDist = maxDouble();
	foreach (FdNode* to, nset)
	{
		double dist = getDistance(n, to);
		if (dist < minDist) minDist = dist;
	}

	return minDist;
}

StrArray2D getIds( FdNodeArray2D nodeArray )
{
	StrArray2D idArray;
	QVector<QString> ids;
	foreach (QVector<FdNode*> ns, nodeArray)
	{
		ids.clear();
		foreach(FdNode* n, ns)	ids.push_back(n->mID);
		idArray.push_back(ids);
	}

	return idArray;
}

QVector<Geom::Segment> detectJointSegments( FdNode* part, PatchNode* panel )
{
	QVector<Geom::Segment> jointSegs;

	if (part->mType == FdNode::PATCH)
	{
		PatchNode* partPatch = (PatchNode*)part;
		Vector3 panelNormal = panel->mPatch.Normal;
		QVector<Geom::Segment> perpEdges = partPatch->mPatch.getPerpEdges(panelNormal);

		Geom::DistSegRect dsr1(perpEdges[0], panel->mPatch);
		Geom::DistSegRect dsr2(perpEdges[1], panel->mPatch);
		if (dsr1.get() < dsr2.get())	jointSegs << perpEdges[0];
		else							jointSegs << perpEdges[1];
	}
	else if (part->mType == FdNode::ROD)
	{
		Geom::Segment distSeg = getDistSegment(part, panel);
		Vector3 p = distSeg.P0;

		Vector3 v1 = panel->mPatch.Axis[0];
		int aid1 = part->mBox.getAxisId(v1);
		double ext1 = part->mBox.getExtent(aid1);

		Vector3 v2 = panel->mPatch.Axis[1];
		int aid2 = part->mBox.getAxisId(v2);
		double ext2 = part->mBox.getExtent(aid2);

		jointSegs << Geom::Segment(p, v1, ext1) << Geom::Segment(p, v2, ext2);
	}

	return jointSegs;
}

double getLocalTime( double globalT, double localStart, double localEnd )
{
	if (globalT < localStart) return 0;
	else if(globalT >= localStart && globalT < localEnd)
		return (globalT - localStart) / (localEnd - localStart);
	else return 1;
}

QVector<double> getEvenDivision( int n, double start /*= 0*/, double end /*= 1*/ )
{
	QVector<double> cp;
	double step = (end - start) / n;
	for (int i = 0; i <= n; i++)
		cp << i * step;

	return cp;
}

Geom::Box fitBox( QVector<Vector3>& pnts, BOX_FIT_METHOD method )
{
	Geom::Box box;

	if (method == FIT_AABB)
	{
		Geom::AABB aabb(pnts);
		box = aabb.box();
	}
	else if (method == FIT_MIN)
	{
		Geom::MinOBB obb(pnts, false);
		box = obb.mMinBox;
	}
	else
	{
		Geom::PcaOBB pcaOBB(pnts);
		box = pcaOBB.minBox;
	}

	return box;
}

PLANE_RELATION relationWithPlane( FdNode* n, Geom::Plane plane, double thr )
{
	double minDist = maxDouble();
	double maxDist = minDouble();
	foreach (Vector3 p, n->mBox.getConnerPoints())
	{
		double dist = plane.signedDistanceTo(p);
		if (dist < minDist) minDist = dist;
		if (dist > maxDist) maxDist = dist;
	}
	double distThr = n->mBox.radius() * thr;

	int minSide;
	if (minDist < -distThr) minSide = -1;
	else if (minDist > distThr) minSide = 1;
	else minSide = 0;

	int maxSide;
	if (maxDist < -distThr) maxSide = -1;
	else if (maxDist > distThr) maxSide = 1;
	else maxSide = 0;

	int sum = minSide + maxSide;
	if (sum >= 1) return POS_PLANE;
	if (sum <= -1) return NEG_PLANE;
	if (minSide == 0 && maxSide == 0) return ON_PLANE;
	else return ISCT_PLANE;
}

bool onPlane( FdNode* n, Geom::Plane& plane )
{
	return (relationWithPlane(n, plane, 0.2) == ON_PLANE);
}

QString getBundleName( const QVector<FdNode*>& nodes )
{
	QString bname;
	foreach (FdNode* n, nodes)
	{
		bname += "+" + n->mID;
	}

	return "(" + bname + ")";
}

Geom::Box getBundleBox( const QVector<FdNode*>& nodes )
{
	QVector<Vector3> points;
	foreach (FdNode* n, nodes)
	{
		points += n->mBox.getConnerPoints();
	}

	return fitBox(points);
}

bool hasIntersection( FdNode* n1, PatchNode* n2 )
{
	// patch - patch
	if (n1->mType == FdNode::PATCH)
	{
		// two rectangles locate on both sides of each other
		PatchNode* pn1 = (PatchNode*)n1;
		Geom::Plane plane1 = pn1->mPatch.getPlane();
		Geom::Plane plane2 = n2->mPatch.getPlane();
		return (relationWithPlane(n1, plane2, 0.1) == ISCT_PLANE
			&& relationWithPlane(n2, plane1, 0.1) == ISCT_PLANE);
	}
	// patch - rod
	else
	{
		// the rod intersect the patch within the patch
		RodNode* rn1 = (RodNode*)n1;
		Geom::Segment sklt = rn1->mRod;

		Geom::Plane plane2 = n2->mPatch.getPlane();
		double dist0 = plane2.signedDistanceTo(sklt.P0);
		double dist1 = plane2.signedDistanceTo(sklt.P1);

		double alpha = dist0 / (dist0 - dist1);
		Vector3 pi = sklt.P0 * alpha + sklt.P1 * (1 - alpha);

		// coordinates of pi in patch
		return n2->mPatch.getRectangle().contains(pi);
	}
}
