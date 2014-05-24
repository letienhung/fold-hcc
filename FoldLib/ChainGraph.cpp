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

		// relative position of two masters
		Vector3 MC2onM1 = master1->mPatch.getProjection(master2->center());
		mMC2Trajectory.set(master2->center(), MC2onM1);
	}

	// setup base orientations
	setupBasisOrientations();

	// fold duration
	// this time interval stops this chain from being folded
	mFoldDuration = TIME_INTERVAL(1, 2);

	// thickness
	top_thk = 1;
	base_thk = 1;
	slave_thk = 1;
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

void ChainGraph::fold( double t )
{
	// free all nodes
	foreach (Structure::Node* n, nodes)
		n->properties["fixed"] = false;

	// fix masters[0]
	mMasters.front()->properties["fixed"] = true;
	mMasters.last()->properties["fixed"] = true;

	// adjust the position of masters[1]
	mMasters.last()->mBox.Center = mMC2Trajectory.getPosition01(t);
	mMasters.last()->createScaffold(true);
	mMasters.last()->addTag(DELETED_TAG); // avoid propagation from master2

	// hinge angle
	// this works only for odd number of splits (even number of even pieces)
	double half_angle = asin(1 - t);
	activeLinks.front()->hinge->angle = half_angle;
	activeLinks.last()->hinge->angle = half_angle;
	for (int i = 1; i < activeLinks.size()-1; i++)
		activeLinks[i]->hinge->angle = 2 * half_angle;

	// restore configuration
	restoreConfiguration();
}

FdGraph* ChainGraph::getKeyframeScaffold( double t )
{
	fold(t);
	return (FdGraph*)this->clone();
}

QVector<FoldOption*> ChainGraph::generateFoldOptions( int nSplits, int nUsedChunks, int nChunks )
{
	QVector<FoldOption*> options;

	// patch chain
	if (mOrigSlave->mType == FdNode::PATCH)
	{
		double chunkSize = 1.0/double(nChunks);
		double usedSize = chunkSize * nUsedChunks;

		// position
		for (int i = 0; i <= nChunks - nUsedChunks; i++)
		{
			double position = chunkSize * i;

			// left
			QString fnid1 = QString("%1:%2_%3_L_%4").arg(mID).arg(nSplits).arg(nUsedChunks).arg(position);
			FoldOption* fn1 = new FoldOption(0, false, usedSize, position, nSplits, fnid1);
			options.push_back(fn1);

			// right
			QString fnid2 = QString("%1:%2_%3_R_%4").arg(mID).arg(nSplits).arg(nUsedChunks).arg(position);
			FoldOption* fn2 = new FoldOption(0, true, usedSize, position, nSplits, fnid2);
			options.push_back(fn2);
		}

	}
	// rod chain
	else
	{
		// root segment id
		for (int j = 0; j < 2; j++)
		{
			// left
			QString fnid1 = QString("%1:%2_L").arg(mID).arg(nSplits);
			FoldOption* fn1 = new FoldOption(j, false, 1.0, 0.0, nSplits, fnid1);
			options.push_back(fn1);

			// right
			QString fnid2 = QString("%1:%2_R").arg(mID).arg(nSplits);;
			FoldOption* fn2 = new FoldOption(j, true, 1.0, 0.0, nSplits, fnid2);
			options.push_back(fn2);
		}
	}	

	return options;
}

void ChainGraph::createSlavePart(FoldOption* fn)
{
	// clear
	foreach (FdNode* n, mParts) removeNode(n->mID);

	// clone original slave
	FdNode* slave = (FdNode*)mOrigSlave->clone();
	Structure::Graph::addNode(slave);

	// self thickness
	if (slave->mType == FdNode::PATCH)
	{
		PatchNode* slave_patch = (PatchNode*)slave;
		int aid = slave->mBox.getAxisId(slave_patch->mPatch.Normal);
		slave->mBox.Extent[aid] = slave_thk;
	}

	// horizontal modification
	Geom::Segment axisSeg = rootJointSegs[fn->jointAxisIdx];
	int haid = slave->mBox.getAxisId(axisSeg.Direction);
	Vector3 boxAxis = slave->mBox.Axis[haid];
	double t0 = fn->position;
	double t1 = fn->position + fn->scale;
	if (dot(axisSeg.Direction, boxAxis) < 0)
		slave->mBox.scaleRange01(haid, 1-t1, 1-t0);
	else slave->mBox.scaleRange01(haid, t0, t1);

	// vertical shrinking caused by master thickness
	double height = mMC2Trajectory.length();
	double top_dt = top_thk / height;
	double base_dt = base_thk / height;
	int vaid = slave->mBox.getAxisId(chainUpSeg.Direction);
	if (dot(slave->mBox.Axis[vaid], chainUpSeg.Direction) > 0)
		slave->mBox.scaleRange01(vaid, base_dt, 1-top_dt);
	else slave->mBox.scaleRange01(vaid, top_dt, 1-base_dt);

	// update mesh and scaffold
	slave->deformMesh();
	slave->createScaffold(true);

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

void ChainGraph::resetHingeLinks()
{
	// remove hinge links
	foreach (Structure::Link* link, links)
		Structure::Graph::removeLink(link);
	hingeLinks.clear();

	// shrink chain up segment for thickness
	double height = mMC2Trajectory.length();
	double top_dt = top_thk / height;
	double base_dt = base_thk / height;
	Geom::Segment chainUpSeg_copy = chainUpSeg;
	chainUpSeg_copy.cropRange01(base_dt, 1 - top_dt);

	// shift root joint segment for thickness
	QVector<Geom::Segment> rootJointSegs_copy = rootJointSegs;
	for (int i = 0; i < rootJointSegs_copy.size(); i++)
		rootJointSegs_copy[i].translate(base_thk * chainUpSeg_copy.Direction);

	// create hinge links between mMaster[0] and mParts[0]
	QVector<FdLink*> links;
	for (int i = 0; i < rootJointSegs_copy.size(); i++)
	{
		Geom::Segment jseg = rootJointSegs_copy[i];
		Vector3 upV = chainUpSeg_copy.Direction;
		Vector3 rV = rootRightVs[i];
		Vector3 axisV = jseg.Direction;
		Vector3 posR = jseg.P0 + slave_thk * rV;
		Vector3 posL = jseg.P1 - slave_thk * rV;
		Hinge* hingeR = new Hinge(mParts[0], mMasters[0], 
			posR, upV,  rV, axisV, jseg.length());
		Hinge* hingeL = new Hinge(mParts[0], mMasters[0], 
			posL, upV, -rV, -axisV, jseg.length());

		FdLink* linkR = new FdLink(mParts[0], mMasters[0], hingeR);
		FdLink* linkL = new FdLink(mParts[0], mMasters[0], hingeL);
		links << linkR << linkL;

		Graph::addLink(linkR);
		Graph::addLink(linkL);
	}
	hingeLinks << links;
	links.clear();

	// create hinge links between two parts in the chain
	int nbParts = mParts.size();
	double step = 2.0 / nbParts;
	for (int i = 1; i < nbParts; i++) // each joint
	{
		Vector3 pos = chainUpSeg_copy.getPosition(-1 + step * i);
		Vector3 deltaV =  pos - chainUpSeg_copy.P0;

		FdNode* part1 = mParts[i];
		FdNode* part2 = mParts[i-1];

		// create a pair of links for each joint segment
		for (int j = 0; j < rootJointSegs_copy.size(); j++)
		{
			Geom::Segment jseg = rootJointSegs_copy[j].translated(deltaV);
			Vector3 upV = chainUpSeg.Direction;
			Vector3 axisV = jseg.Direction;
			Vector3 rV = rootRightVs[j];
			Vector3 posR = jseg.P0 + slave_thk * rV;
			Vector3 posL = jseg.P1 - slave_thk * rV;
			Hinge* hingeR = new Hinge(part1, part2, 
				posR, upV, -upV, axisV, jseg.length());
			Hinge* hingeL = new Hinge(part1, part2, 
				posL, upV, -upV, -axisV, jseg.length());

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
		for (int i = 0; i < rootJointSegs_copy.size(); i++)
		{
			Geom::Segment jseg = rootJointSegs_copy[i].translated(chainUpSeg_copy.P1 - chainUpSeg_copy.P0);
			Vector3 upV = chainUpSeg.Direction;
			Vector3 rV = rootRightVs[i];
			Vector3 axisV = jseg.Direction;
			Vector3 posR = jseg.P1 + slave_thk * rV;
			Vector3 posL = jseg.P0 - slave_thk * rV;
			Hinge* hingeR = new Hinge(mParts.last(), mMasters[1], 
				posR, -upV,  rV, -axisV, jseg.length());
			Hinge* hingeL = new Hinge(mParts.last(), mMasters[1], 
				posL, -upV, -rV, axisV, jseg.length());

			FdLink* linkR = new FdLink(mParts.last(), mMasters[1], hingeR);
			FdLink* linkL = new FdLink(mParts.last(), mMasters[1], hingeL);
			links << linkR << linkL;

			Graph::addLink(linkR);
			Graph::addLink(linkL);
		}

		hingeLinks << links;
	}
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

void ChainGraph::applyFoldOption( FoldOption* fn)
{
	// delete chain if fold option is null
	if (fn == NULL) {
		addTag(DELETED_TAG);
		std::cout << "Chain: " << mID.toStdString() << " is deleted.\n";
		return;
	}
	
	// clear tag
	removeTag(DELETED_TAG); 

	createSlavePart(fn);
	mParts = FdGraph::split(mOrigSlave->mID, generateCutPlanes(fn->nSplits));
	sortChainParts();
	resetHingeLinks();
	setupActiveLinks(fn);
}

void ChainGraph::setFoldDuration( double t0, double t1 )
{
	if (t0 > t1) std::swap(t0, t1);
	t0 += ZERO_TOLERANCE_LOW;
	t1 -= ZERO_TOLERANCE_LOW;

	mFoldDuration = TIME_INTERVAL(t0, t1);
}
