#include "FdUtility.h"

#include "RodNode.h"
#include "PatchNode.h"
#include "FdGraph.h"

#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "DistRectRect.h"
#include "Numeric.h"
#include "MinOBB.h"
#include "PcaOBB.h"

#include "RootFinder.h"

#include <iomanip>

FdNodeArray2D getConnectedGroups( QVector<FdNode*> nodes, double disThr )
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
	StrArray2D idArray2D;
	foreach (QVector<FdNode*> ns, nodeArray) idArray2D << getIds(ns);
	return idArray2D;
}

QVector<QString> getIds( QVector<FdNode*> nodes )
{
	QVector<QString> ids;
	foreach(FdNode* n, nodes)	ids.push_back(n->mID);
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

double getLocalTime( double globalT, Interval itv )
{
	if (globalT <= itv.first) return 0;
	else if(globalT > itv.first && globalT < itv.second)
		return (globalT - itv.first) / (itv.second - itv.first);
	else return 1;
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

bool hasIntersection( FdNode* slave, PatchNode* master, double thr )
{
	if (getDistance(slave, master) > thr) return false;

	// check whether slave locates on both sides of master
	double leftEnd = maxDouble();
	double rightEnd = -maxDouble();
	foreach (Vector3 p, slave->mBox.getConnerPoints())
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

bool overlap( Interval itv1, Interval itv2 )
{
	return within(itv1.first, itv2) || within(itv1.second, itv2) ||
		within(itv2.first, itv1) || within(itv2.second, itv1);
}

bool within( double t, Interval itv )
{
	return inRange(t, itv.first, itv.second);
}

QVector<PatchNode*> getAllMasters( FdGraph* scaffold )
{
	QVector<PatchNode*> masters;
	foreach(Structure::Node* n, scaffold->getNodesWithTag(MASTER_TAG))
		masters << (PatchNode*)n;

	return masters;
}

FdGraph* combineDecomposition( QVector<FdGraph*> decmps, QString baseMid, 
								QMap<QString, QSet<int> >& masterDecmpMap )
{
	// trivial combination
	if (decmps.size() == 1)
	{
		FdGraph * f = decmps.front();
		if(!f) return NULL;

		FdGraph * fdg = (FdGraph*)f->clone();
		return fdg;
	}

	// combined tags
	QMap<QString, bool> masterCombined;
	foreach (QString mid, masterDecmpMap.keys()) 
		masterCombined[mid] = false;
	QVector<bool> decmpCombined(decmps.size(), false);
	for (int i = 0; i < decmps.size(); i++)
		if (decmps[i] == NULL) decmpCombined[i] = true;

	// start from base master
	int base_decmp_id = -1;
	foreach (int bdid, masterDecmpMap[baseMid])
	{
		if (decmps[bdid] != NULL)
		{
			base_decmp_id = bdid;
			break;
		}
	}
	if (base_decmp_id < 0) return NULL;
	
	// create scaffold for combination
	FdGraph* combination = new FdGraph();
	FdGraph* base_decmp = decmps[base_decmp_id];
	PatchNode* baseMaster = (PatchNode*)base_decmp->getNode(baseMid);
	combination->Structure::Graph::addNode(baseMaster);
	masterCombined[baseMid] = true;

	// prorogation
	QQueue<QString> activeMids;
	activeMids.enqueue( baseMid );
	while (!activeMids.isEmpty())
	{
		// set current to an active master
		QString curr_mid = activeMids.dequeue();
		PatchNode* currMaster = (PatchNode*)combination->getNode(curr_mid);
		Vector3 currPos = currMaster->center();

		// combine decompositions containing current master
		foreach (int decmp_id, masterDecmpMap[curr_mid])
		{
			// skip combined decmp
			if (decmpCombined[decmp_id]) continue;

			// translate
			FdGraph* decmp = decmps[decmp_id];
			PatchNode* oldMaster = (PatchNode*)decmp->getNode(curr_mid);
			Vector3 delta = currPos - oldMaster->center();
			decmp->translate(delta);

			// combine parts from decomposition
			foreach (Structure::Node* n, decmp->nodes)
			{
				// master nodes
				if (n->hasTag(MASTER_TAG))
				{
					if (!masterCombined[n->mID])
					{
						// combine unvisited masters
						combination->Structure::Graph::addNode(n->clone());
						masterCombined[n->mID] = true;

						// store as active
						activeMids.enqueue(n->mID);
					}
				}
				// clone slave nodes
				else 
				{
					combination->Structure::Graph::addNode(n->clone());
				}
			}

			// mark current decmp
			decmpCombined[decmp_id] = true;
		}
	}

	return combination;
}

int nbMasters( FdGraph* scaffold )
{
	return getAllMasters(scaffold).size();
}

QVector<QString> getAllMasterIds( FdGraph* scaffold )
{
	QVector<QString> mids;
	foreach (PatchNode* m, getAllMasters(scaffold))
		mids << m->mID;

	return mids;
}

Geom::Rectangle2 computeAABB2D( QVector<Vector2> &pnts )
{
	// compute extent along x and y
	double minX = maxDouble();
	double maxX = -maxDouble();
	double minY = maxDouble();
	double maxY = -maxDouble();
	foreach (Vector2 p, pnts)
	{
		if (p.x() < minX) minX = p.x();
		if (p.x() > maxX) maxX = p.x();

		if (p.y() < minY) minY = p.y();
		if (p.y() > maxY) maxY = p.y();
	}

	// create rect
	QVector<Vector2> conners;
	conners << Vector2(minX, minY) << Vector2(maxX, minY) << Vector2(maxX, maxY) << Vector2(minX, maxY);
	return Geom::Rectangle2(conners);
}

bool passed( double t, Interval itv )
{
	return t >= itv.second;
}

QMap<QString, double> getTimeStampsNormalized( QVector<FdNode*> nodes, Vector3 v, double &tScale )
{
	QMap<QString, double> timeStamps;

	// time line
	Geom::Line timeLine(Vector3(0, 0, 0), v);

	// position on time line
	double minT = maxDouble();
	double maxT = -maxDouble();
	Geom::AABB aabb;
	foreach (FdNode* n, nodes)
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
	foreach (QString mid, timeStamps.keys())
	{
		timeStamps[mid] = ( timeStamps[mid] - minT ) * scaler;
	}

	return timeStamps;
}

QMap<QString, double> getTimeStampsNormalized( QVector<PatchNode*> pnodes, Vector3 v, double &tScale )
{
	QVector<FdNode*> nodes;
	foreach (PatchNode* pn, pnodes) nodes << pn;
	return getTimeStampsNormalized(nodes, v, tScale);
}

bool extendRectangle2D( Geom::Rectangle2& rect, QVector<Vector2> &pnts )
{
	// shrink seed rect to avoid pnts on edges
	rect.Extent *= 0.9;

	// do nothing if seed rect contains any pnts
	foreach (Vector2 p, pnts) 
		if (rect.contains(p)) 
			return false;

	// coordinates in the frame of seed rect
	QVector<Vector2> pnts_coord;
	foreach (Vector2 p, pnts) pnts_coord << rect.getCoordinates(p);

	// extend along x
	double left = -maxDouble();
	double right = maxDouble();
	foreach (Vector2 pc, pnts_coord)
	{
		// keep the extent along y
		if (inRange(pc.y(), -1, 1))
		{
			double x = pc.x();
			// tightest bound on left
			if (x < 0 && x > left) left = x;
			// tightest bound on right
			if (x > 0 && x < right) right = x;
		}
	}

	// extend along y
	//double epsilon = 2 * ZERO_TOLERANCE_LOW;
	//double epsilon = 0.01 * (right - left);
	double bottom = -maxDouble();
	double top = maxDouble();
	foreach (Vector2 pc, pnts_coord)
	{
		// keep the extent along x
		//if (inRange(pc.x(), left + epsilon, right - epsilon))
		if (inRange(pc.x(), -1, 1))
		{
			double y = pc.y();
			// tightest bound on left
			if (y < 0 && y > bottom) bottom = y;
			// tightest bound on right
			if (y > 0 && y < top) top = y;
		}
	}

	// set up box
	QVector<Vector2> new_conners;
	new_conners << rect.getPosition(Vector2(left, bottom))
				<< rect.getPosition(Vector2(right, bottom))
				<< rect.getPosition(Vector2(right, top))
				<< rect.getPosition(Vector2(left, top));
	rect =  Geom::Rectangle2(new_conners);
	return true;
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
