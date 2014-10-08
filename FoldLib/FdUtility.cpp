#include "FdUtility.h"

#include "RodNode.h"
#include "PatchNode.h"
#include "Scaffold.h"

#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "DistRectRect.h"
#include "Numeric.h"
#include "MinOBB.h"
#include "PcaOBB.h"

#include "RootFinder.h"

#include <iomanip>

FdNodeArray2D getConnectedGroups( QVector<ScaffNode*> nodes, double disThr )
{
	FdNodeArray2D clusters;
	if (nodes.isEmpty()) return clusters;

	// tags used for searching
	QVector<bool> visited(nodes.size(), false);
	int nbVisited = 0;

	// initial current graph
	QVector<ScaffNode*> curr_g;
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

Geom::Segment getDistSegment( ScaffNode* n1, ScaffNode* n2 )
{
	Geom::Segment ds;

	if (n1->mType == ScaffNode::ROD)
	{
		RodNode* node1 = (RodNode*)n1;
		Geom::Segment rod1 = node1->mRod;

		// rod-rod
		if (n2->mType == ScaffNode::ROD)
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
		if (n2->mType == ScaffNode::ROD)
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

double getDistance( ScaffNode* n1, ScaffNode* n2 )
{
	return getDistSegment(n1, n2).length();
}

double getDistance( ScaffNode* n, QVector<ScaffNode*> nset )
{
	double minDist = maxDouble();
	for (ScaffNode* to : nset)
	{
		double dist = getDistance(n, to);
		if (dist < minDist) minDist = dist;
	}

	return minDist;
}

StrArray2D getIds( FdNodeArray2D nodeArray )
{
	StrArray2D idArray2D;
	for (QVector<ScaffNode*> ns : nodeArray) idArray2D << getIds(ns);
	return idArray2D;
}

QVector<QString> getIds( QVector<ScaffNode*> nodes )
{
	QVector<QString> ids;
	for (ScaffNode* n : nodes)	ids.push_back(n->mID);
	return ids;
}

Geom::Segment detectJointSegment( PatchNode* slave, PatchNode* master )
{
	Vector3 panelNormal = master->mPatch.Normal;
	QVector<Geom::Segment> perpEdges = slave->mPatch.getPerpEdges(panelNormal);

	Geom::DistSegRect dsr1(perpEdges[0], master->mPatch);
	Geom::DistSegRect dsr2(perpEdges[1], master->mPatch);
	Geom::Segment jointSeg = (dsr1.get() < dsr2.get()) ? 
							perpEdges[0] : perpEdges[1];

	return jointSeg;
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

PLANE_RELATION relationWithPlane( ScaffNode* n, Geom::Plane plane, double thr )
{
	double minDist = maxDouble();
	double maxDist = minDouble();
	for (Vector3 p : n->mBox.getConnerPoints())
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

bool onPlane( ScaffNode* n, Geom::Plane& plane )
{
	return (relationWithPlane(n, plane, 0.2) == ON_PLANE);
}

QString getBundleName( const QVector<ScaffNode*>& nodes )
{
	QString bname;
	for (ScaffNode* n : nodes)
	{
		bname += "+" + n->mID;
	}

	return "(" + bname + ")";
}

Geom::Box getBundleBox( const QVector<ScaffNode*>& nodes )
{
	QVector<Vector3> points;
	for (ScaffNode* n : nodes)
	{
		points += n->mBox.getConnerPoints();
	}

	return fitBox(points);
}

bool hasIntersection( ScaffNode* slave, PatchNode* master, double thr )
{
	if (getDistance(slave, master) > thr) return false;

	// check whether slave locates on both sides of master
	double leftEnd = maxDouble();
	double rightEnd = -maxDouble();
	for (Vector3 p : slave->mBox.getConnerPoints())
	{
		double sd = master->mPatch.getPlane().signedDistanceTo(p);
		if (sd < leftEnd) leftEnd = sd;
		if (sd > rightEnd) rightEnd = sd;
	}

	// one side
	if (leftEnd * rightEnd >= 0) return false;
	// two sides
	else
	{
		double shorterEnd = Min(fabs(leftEnd), fabs(rightEnd));
		return shorterEnd > thr;
	}
}

QMap<QString, double> getTimeStampsNormalized( QVector<ScaffNode*> nodes, Vector3 v, double &tScale )
{
	QMap<QString, double> timeStamps;

	// time line
	Geom::Line timeLine(Vector3(0, 0, 0), v);

	// position on time line
	double minT = maxDouble();
	double maxT = -maxDouble();
	Geom::AABB aabb;
	for (ScaffNode* n : nodes)
	{
		double t = timeLine.getProjTime(n->center());
		timeStamps[n->mID] = t;

		if (t < minT) minT = t;
		if (t > maxT) maxT = t;

		aabb.add(n->mBox.getConnerPoints());
	}

	// set up tScale
	tScale = maxT - minT;
	double scaler;
	if (tScale < 0.1 * aabb.radius())
		scaler = 0;
	else
		scaler = 1 / tScale;

	// normalize time stamps
	for (QString mid : timeStamps.keys())
	{
		timeStamps[mid] = ( timeStamps[mid] - minT ) * scaler;
	}

	return timeStamps;
}

QMap<QString, double> getTimeStampsNormalized( QVector<PatchNode*> pnodes, Vector3 v, double &tScale )
{
	QVector<ScaffNode*> nodes;
	for (PatchNode* pn : pnodes) nodes << pn;
	return getTimeStampsNormalized(nodes, v, tScale);
}

void print( Vector3 v )
{
	std::cout << std::setprecision(3) << v[0] << "  " << v[1] << "  " << v[2] << "  \n";
}

void print( Geom::Box box)
{
	std::cout << "Center = ";
	print(box.Center);
	std::cout << "Extent = ";
	print(box.Extent);
	std::cout << "X = ";
	print(box.Axis[0]);
	std::cout << "Y = ";
	print(box.Axis[1]);
	std::cout << "Z = ";
	print(box.Axis[2]);
}

QVector<double> findRoots( QVector<double>& coeff )
{
	QVector<double> roots;

	// remove zero coeff
	int n;
	for (n = 0; n < coeff.size(); n++)
		if (!RootFinder::is_zero(coeff[n])) break;
	coeff.remove(0, n);

	// reverse order
	if (coeff.size() >= 2)
	{
		std::vector<double> coeff_std;
		for (int i = coeff.size()-1; i >= 0; i--) coeff_std.push_back(coeff[i]);

		std::vector<double> roots_std = RootFinder::find_roots(coeff_std);
		roots = QVector<double>::fromStdVector(roots_std);
	}

	return roots;
}

QVector<double> findRoots( double a, double b, double c )
{
	return findRoots(QVector<double>() << a << b << c);
}

QVector<double> findRoots( double a, double b, double c, double d, double e )
{
	return findRoots(QVector<double>() << a << b << c << d << e);
}
