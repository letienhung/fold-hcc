#include "ChainGraph.h"
#include "FdUtility.h"
#include "RodNode.h"
#include "Numeric.h"

ChainGraph::ChainGraph( FdNode* slave, PatchNode* master1, PatchNode* master2)
	: FdGraph(slave->mID)
{
	// clone parts
	mOrigSlave = (FdNode*)slave->clone();

	mParts << (FdNode*)mOrigSlave->clone();
	Structure::Graph::addNode(mParts[0]);

	mMasters << (PatchNode*)master1->clone();
	Graph::addNode(mMasters[0]);
	if (master2)
	{
		mMasters << (PatchNode*)master2->clone();
		Graph::addNode(mMasters[1]);
	}

	// setup base orientations
	setupBasisOrientations();
}

void ChainGraph::setupBasisOrientations()
{
	// detect hinges
	rootJointSegs = detectJointSegments(mOrigSlave, mMasters[0]);

	// upSeg
	Geom::Segment jointSeg = rootJointSegs[0];
	Vector3 origin = jointSeg.P0;
	if (mOrigSlave->mType == FdNode::PATCH)
	{
		PatchNode* partPatch = (PatchNode*)mOrigSlave;
		QVector<Geom::Segment> edges = partPatch->mPatch.getPerpEdges(jointSeg.Direction);
		chainUpSeg = edges[0].contains(origin) ? edges[0] : edges[1];
	}
	else
	{
		RodNode* partRod = (RodNode*)mOrigSlave;
		chainUpSeg = partRod->mRod;
	}
	if (chainUpSeg.getProjCoordinates(origin) > 0) chainUpSeg.flip();

	// righV
	foreach (Geom::Segment rjs, rootJointSegs)
	{
		Vector3 crossAxisV1 = cross(rjs.Direction, chainUpSeg.Direction);
		Vector3 rV = mMasters[0]->mPatch.getProjectedVector(crossAxisV1);

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
	foreach(FdNode* n, mMasters)
		knodes << n->clone();

	return knodes;
}

// N is the number of cut planes
QVector<Geom::Plane> ChainGraph::generateCutPlanes( int N )
{
	// plane of panel0
	Geom::Plane plane0 = mMasters[0]->mPatch.getPlane();
	if (plane0.whichSide(mOrigSlave->center()) < 0) plane0.flip();

	// deltaV to shift up
	double step = getLength() / (N + 1);

	QVector<Geom::Plane> cutPlanes;
	for (int i = 0; i < N; i++)
	{
		Vector3 deltaV = (i+1) * step * plane0.Normal;
		cutPlanes << plane0.translated(deltaV);
	}

	return cutPlanes;
}

void ChainGraph::createChain( int N )
{
	// remove original chain parts
	foreach (FdNode* n, mParts) removeNode(n->mID);

	// clone original part
	// add to graph and split
	FdNode* origPart = (FdNode*)mOrigSlave->clone();
	Structure::Graph::addNode(origPart);
	mParts = FdGraph::split(mOrigSlave->mID, generateCutPlanes(N - 1));
	
	// reset hinge links
	sortChainParts();
	resetHingeLinks();
}

double ChainGraph::getLength()
{
	return chainUpSeg.length();
}

void ChainGraph::sortChainParts()
{
	QMap<double, FdNode*> distPartMap;

	Geom::Plane panel_plane = mMasters[0]->mPatch.getPlane();
	foreach(FdNode* n, mParts)
	{
		double dist = panel_plane.signedDistanceTo(n->center());
		distPartMap[fabs(dist)] = n;
	}

	mParts = distPartMap.values().toVector();
}

void ChainGraph::fold( double t )
{
	// fix panels[0] but free all others
	foreach (Structure::Node* n, nodes)
		n->properties["fixed"] = false;
	mMasters[0]->properties["fixed"] = true;

	// hinge angle
	foreach(FdLink* alink, activeLinks)
		alink->hinge->setAngleByTime(t);

	// apply folding
	restoreConfiguration();
}

void ChainGraph::setupActiveLinks( FoldingNode* fn )
{
	activeLinks.clear();

	int hidx_m = fn->hingeIdx;
	int hidx_v = (hidx_m % 2) ? hidx_m - 1 : hidx_m + 1;
	for (int i = 0; i < hingeLinks.size(); i++)
	{
		// activate corresponding hinges
		foreach(FdLink* l, hingeLinks[i])
			l->properties["active"] = false;

		int hidx = (i % 2) ? hidx_v : hidx_m;
		FdLink* activeLink = hingeLinks[i][hidx];
		activeLink->properties["active"] = true;
		activeLinks << activeLink;
	}
}

void ChainGraph::resetHingeLinks()
{
	// remove hinge links
	foreach (Structure::Link* link, links)
		Structure::Graph::removeLink(link);
	hingeLinks.clear();

	// create hinge links between mPanels[0] and mParts[0]
	QVector<FdLink*> links;
	for (int i = 0; i < rootJointSegs.size(); i++)
	{
		Geom::Segment jseg = rootJointSegs[i];
		Vector3 upV = chainUpSeg.Direction;
		Vector3 rV = rootRightVs[i];
		Vector3 axisV = jseg.Direction;
		Hinge* hingeR = new Hinge(mParts[0], mMasters[0], 
			jseg.P0, upV,  rV, axisV, jseg.length());
		Hinge* hingeL = new Hinge(mParts[0], mMasters[0], 
			jseg.P1, upV, -rV, -axisV, jseg.length());

		FdLink* linkR = new FdLink(mParts[0], mMasters[0], hingeR);
		FdLink* linkL = new FdLink(mParts[0], mMasters[0], hingeL);
		links << linkR << linkL;

		Graph::addLink(linkR);
		Graph::addLink(linkL);
	}
	hingeLinks << links;
	links.clear();

	// create hinge links between two rods in the chain
	int nbRods = mParts.size();
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
	if (mMasters.size() == 2)
	{
		for (int i = 0; i < rootJointSegs.size(); i++)
		{
			Geom::Segment jseg = rootJointSegs[i].translated(chainUpSeg.P1 - chainUpSeg.P0);
			Vector3 upV = chainUpSeg.Direction;
			Vector3 rV = rootRightVs[i];
			Vector3 axisV = jseg.Direction;
			Hinge* hingeR = new Hinge(mParts.last(), mMasters[1], 
				jseg.P1, -upV,  rV, -axisV, jseg.length());
			Hinge* hingeL = new Hinge(mParts.last(), mMasters[1], 
				jseg.P0, -upV, -rV, axisV, jseg.length());

			FdLink* linkR = new FdLink(mParts.last(), mMasters[1], hingeR);
			FdLink* linkL = new FdLink(mParts.last(), mMasters[1], hingeL);
			links << linkR << linkL;

			Graph::addLink(linkR);
			Graph::addLink(linkL);
		}

		hingeLinks << links;
	}
}

void ChainGraph::shrinkChainAlongJoint(double t0, double t1)
{
	Vector3 jointV = rootJointSegs.front().Direction;
	foreach(FdNode* part, mParts)
	{
		int aid = part->mBox.getAxisId(jointV);
		part->mBox.scale(aid, t0, t1);
		part->deformMesh();
		part->createScaffold();
	}
}
