#include "ChainGraph.h"
#include "FdUtility.h"
#include "RodNode.h"
#include "Numeric.h"

ChainGraph::ChainGraph( FdNode* part, PatchNode* panel1, PatchNode* panel2)
	: FdGraph(part->mID)
{
	// clone parts
	mOrigPart = (FdNode*)part->clone();
	mParts << (FdNode*)mOrigPart->clone();
	Structure::Graph::addNode(mParts[0]);

	mPanels << (PatchNode*)panel1->clone();
	mPanels[0]->properties["isCtrlPanel"] = true;
	Graph::addNode(mPanels[0]);

	if (panel2)
	{
		mPanels << (PatchNode*)panel2->clone();
		mPanels[1]->properties["isCtrlPanel"] = true;
		Graph::addNode(mPanels[1]);
	}

	// detect hinges
	rootJointSegs = detectJointSegments(mOrigPart, mPanels[0]);

	// upSeg
	Geom::Segment jointSeg = rootJointSegs[0];
	Vector3 origin = jointSeg.P0;
	if (mOrigPart->mType == FdNode::PATCH)
	{
		PatchNode* partPatch = (PatchNode*)mOrigPart;
		QVector<Geom::Segment> edges = partPatch->mPatch.getPerpEdges(jointSeg.Direction);
		chainUpSeg = edges[0].contains(origin) ? edges[0] : edges[1];
	}
	else
	{
		RodNode* partRod = (RodNode*)mOrigPart;
		chainUpSeg = partRod->mRod;
	}
	if (chainUpSeg.getProjCoordinates(origin) > 0) chainUpSeg.flip();

	// righV
	foreach (Geom::Segment rjs, rootJointSegs)
	{
		Vector3 crossAxisV1 = cross(rjs.Direction, chainUpSeg.Direction);
		Vector3 rV = mPanels[0]->mPatch.getProjectedVector(crossAxisV1);

		rootRightVs.push_back(rV.normalized());
	}
}

QVector<Structure::Node*> ChainGraph::getKeyframeParts( double t )
{
	// apply fold
	fold(t);

	// clone nodes
	QVector<Structure::Node*> knodes;
	foreach(FdNode* n, mParts)
		knodes << n->clone();

	return knodes;
}

QVector<Structure::Node*> ChainGraph::getKeyFramePanels( double t )
{
	// apply fold
	fold(t);

	// clone nodes
	QVector<Structure::Node*> knodes;
	foreach(FdNode* n, mPanels)
		knodes << n->clone();

	return knodes;
}

// N is the number of cut planes
QVector<Geom::Plane> ChainGraph::generateCutPlanes( int N )
{
	// plane of panel0
	Geom::Plane plane0 = mPanels[0]->mPatch.getPlane();
	if (plane0.whichSide(mOrigPart->center()) < 0) plane0.flip();

	// deltaV to shift up
	double step = getHeight() / (N + 1);
	Vector3 deltaV = step * plane0.Normal;

	QVector<Geom::Plane> cutPlanes;
	for (int i = 0; i < N; i++)
	{
		cutPlanes << plane0.translated(deltaV);
	}

	return cutPlanes;
}

void ChainGraph::splitChain( int N )
{
	// remove original chain parts
	foreach (FdNode* n, mParts) removeNode(n->mID);

	// clone original part and add to graph
	Structure::Graph::addNode(mOrigPart->clone());

	// split part
	mParts = FdGraph::split(mOrigPart->mID, generateCutPlanes(N - 1));
	sortChainParts();

	// create hinge links between mPanels[0] and mParts[0]
	QVector<FdLink*> links;
	for (int i = 0; i < rootJointSegs.size(); i++)
	{
		Geom::Segment jseg = rootJointSegs[i];
		Vector3 upV = chainUpSeg.Direction;
		Vector3 rV = rootRightVs[i];
		Vector3 axisV = jseg.Direction;
		Hinge* hingeR = new Hinge(mParts[0], mPanels[0], 
			jseg.P0, upV,  rV, axisV, jseg.length());
		Hinge* hingeL = new Hinge(mParts[0], mPanels[0], 
			jseg.P1, upV, -rV, -axisV, jseg.length());

		FdLink* linkR = new FdLink(mParts[0], mPanels[0], hingeR);
		FdLink* linkL = new FdLink(mParts[0], mPanels[0], hingeL);
		links << linkR << linkL;

		Graph::addLink(linkR);
		Graph::addLink(linkL);
	}
	hingeLinks << links;
	links.clear();

	// create hinge links between two rods in the chain
	nbRods = 2;
	double step = 2.0 / nbRods;
	for (int i = 1; i < nbRods; i++) // each joint
	{
		Vector3 pos = chainUpSeg.getPosition(-1 + step * i);
		Vector3 deltaV =  pos - chainUpSeg.P0;

		FdNode* part1 = mParts[i];
		FdNode* part2 = mParts[i-1];

		// create a pair of links for each joint seg
		for (int j = 0; j < rootJointSegs.size(); j++)
		{
			Geom::Segment jseg = rootJointSegs[j].translated(deltaV);
			Vector3 upV = chainUpSeg.Direction;
			Vector3 axisV = jseg.Direction;
			Hinge* hingeR = new Hinge(part1, part2, 
				jseg.P0, upV, -upV, axisV, jseg.length());
			Hinge* hingeL = new Hinge(part1, part2, 
				jseg.P1, upV, -upV, -axisV, jseg.length());

			FdLink* linkR = new FdLink(part1, part2, hingeR);
			FdLink* linkL = new FdLink(part1, part2, hingeL);

			links << linkR << linkL;

			Graph::addLink(linkR);
			Graph::addLink(linkL);
		}

		hingeLinks << links;
		links.clear();
	}

	// create hinge links between mPanels[1] and mParts[1]
	for (int i = 0; i < rootJointSegs.size(); i++)
	{
		Geom::Segment jseg = rootJointSegs[i].translated(chainUpSeg.P1 - chainUpSeg.P0);
		Vector3 upV = chainUpSeg.Direction;
		Vector3 rV = rootRightVs[i];
		Vector3 axisV = jseg.Direction;
		Hinge* hingeR = new Hinge(mParts.last(), mPanels[1], 
			jseg.P1, -upV,  rV, -axisV, jseg.length());
		Hinge* hingeL = new Hinge(mParts.last(), mPanels[1], 
			jseg.P0, -upV, -rV, axisV, jseg.length());

		FdLink* linkR = new FdLink(mParts.last(), mPanels[1], hingeR);
		FdLink* linkL = new FdLink(mParts.last(), mPanels[1], hingeL);
		links << linkR << linkL;

		Graph::addLink(linkR);
		Graph::addLink(linkL);
	}

	hingeLinks << links;
}

double ChainGraph::getHeight()
{
	int aid = mOrigPart->mBox.getClosestAxisId(mPanels[0]->mPatch.Normal);
	Geom::Segment sklt = mOrigPart->mBox.getSkeleton(aid);

	Geom::Plane plane0 = mPanels[0]->mPatch.getPlane();
	double dist1 = plane0.distanceTo(sklt.P0);
	double dist2 = plane0.distanceTo(sklt.P1);

	return Max(dist1, dist2);
}

void ChainGraph::sortChainParts()
{
	QMap<double, FdNode*> distPartMap;

	Geom::Plane panel_plane = mPanels[0]->mPatch.getPlane();
	foreach(FdNode* n, mParts)
	{
		double dist = panel_plane.signedDistanceTo(n->center());
		distPartMap[fabs(dist)] = n;
	}

	mParts = distPartMap.values().toVector();
}
