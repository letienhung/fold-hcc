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

	// fold duration
	// this time interval stops this chain from being folded
	mFoldDuration = TIME_INTERVAL(1, 2);
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

FdGraph* ChainGraph::getKeyframeScaffold( double t )
{
	// fix masters[0] but free all others
	foreach (Structure::Node* n, nodes)
		n->properties["fixed"] = false;
	mMasters[0]->properties["fixed"] = true;

	// hinge angle
	foreach(FdLink* alink, activeLinks)
		alink->hinge->setAngleByTime(t);

	// restore configuration
	restoreConfiguration();

	return (FdGraph*)this->clone();
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

void ChainGraph::setupActiveLinks( FoldOption* fn )
{
	// clear
	activeLinks.clear();

	// hinge index for given joint axis
	int hidx_right = 2 * fn->jointAxisIdx;
	int hidx_left = hidx_right + 1;

	// right side: even(right) odd(left)
	// left side: reverse
	int hidx_even = (fn->rightSide)? hidx_right : hidx_left;
	int hidx_odd = (fn->rightSide)? hidx_left : hidx_right;

	// set hinge for each joint
	for (int i = 0; i < hingeLinks.size(); i++)
	{
		// inactive all hinges
		foreach(FdLink* l, hingeLinks[i])
			l->properties["active"] = false;

		int hidx = (i % 2) ? hidx_odd : hidx_even;
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

	// create hinge links between mMaster[0] and mParts[0]
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

	// create hinge links between mMasters[1] and mParts.last()
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

QVector<FoldOption*> ChainGraph::generateFoldOptions( int nbSplit0, int nbSplit1, int nbScales )
{
	QVector<FoldOption*> options;

	// #splits
	for (int n = nbSplit0; n <= nbSplit1; n++)
	{
		// patch chain
		if (mOrigSlave->mType == FdNode::PATCH)
		{
			// shrink
			double step = 1.0/double(nbScales);
			for (int i = 1; i <= nbScales; i++)
			{
				double scale = step * i;
				// position
				for (int j = 0; j <= nbScales - i; j++)
				{
					double position = step * j;
					// left
					QString fnid1 = this->mID + "_" + QString::number(options.size());
					FoldOption* fn1 = new FoldOption(0, false, scale, position, n, fnid1);
					options.push_back(fn1);

					// right
					QString fnid2 = this->mID + "_" + QString::number(options.size());
					FoldOption* fn2 = new FoldOption(0, true, scale, position, n, fnid2);
					options.push_back(fn2);
				}
			}
		}
		// rod chain
		else
		{
			// root segment id
			for (int j = 0; j < 2; j++)
			{
				// left
				QString fnid1 = this->mID + "_" + QString::number(options.size());
				FoldOption* fn1 = new FoldOption(j, false, 1.0, 0.0, n, fnid1);
				options.push_back(fn1);

				// right
				QString fnid2 = this->mID + "_" + QString::number(options.size());
				FoldOption* fn2 = new FoldOption(j, true, 1.0, 0.0, n, fnid2);
				options.push_back(fn2);
			}
		}	
	}


	return options;
}

void ChainGraph::applyFoldOption( FoldOption* fn )
{
	// remove original chain parts
	foreach (FdNode* n, mParts) removeNode(n->mID);

	// clone original part
	FdNode* origPart = (FdNode*)mOrigSlave->clone();
	Structure::Graph::addNode(origPart);

	// shrink
	if (fn->scale < 1.0 - ZERO_TOLERANCE_LOW)
	{
		Vector3 jointV = rootJointSegs[fn->jointAxisIdx].Direction;
		int aid = origPart->mBox.getAxisId(jointV);
		origPart->mBox.scaleRand01(aid, fn->position, fn->position + fn->scale);
		origPart->deformMesh();
		origPart->createScaffold();
	}

	// split
	mParts = FdGraph::split(mOrigSlave->mID, generateCutPlanes(fn->nbsplit));

	// sort
	sortChainParts();

	// reset hinge links
	resetHingeLinks();
	setupActiveLinks(fn);
}
