#include "ChainGraph.h"
#include "FdUtility.h"
#include "RodNode.h"
#include "Numeric.h"

ChainGraph::ChainGraph( FdNode* slave, PatchNode* base, PatchNode* top)
	: FdGraph(slave->mID)
{

	// slave
	if (slave->mType == FdNode::ROD)
	{
		// convert slave into patch if need
		RodNode* slaveRod = (RodNode*)slave;
		Vector3 rodV = slaveRod->mRod.Direction;
		Vector3 baseV = base->mPatch.Normal;
		Vector3 crossRodVBaseV = cross(rodV, baseV);
		Vector3 slavePatchNorm;
		if (crossRodVBaseV.norm() < 0.1)
			slavePatchNorm = baseMaster->mPatch.Axis[0];
		else
			slavePatchNorm = cross(crossRodVBaseV, rodV);
		slavePatchNorm.normalize();
		
		mOrigSlave = new PatchNode(slaveRod, slavePatchNorm);
	}
	else
		mOrigSlave = (PatchNode*)slave->clone();

	mParts << (PatchNode*)mOrigSlave->clone();
	Structure::Graph::addNode(mParts[0]);

	// masters
	baseMaster = (PatchNode*)base->clone();
	topMaster = (PatchNode*)top->clone();
	Graph::addNode(baseMaster);
	Graph::addNode(topMaster);

	Vector3 MC2onM1 = base->mPatch.getProjection(top->center());
	mMC2Trajectory.set(top->center(), MC2onM1);

	// joint
	baseJoint = detectJointSegment(mOrigSlave, baseMaster);

	// upSeg
	Vector3 baseP0 = baseJoint.P0;
	QVector<Geom::Segment> edges = mOrigSlave->mPatch.getPerpEdges(baseJoint.Direction);
	UpSeg = edges[0].contains(baseP0) ? edges[0] : edges[1];
	if (UpSeg.getProjCoordinates(baseP0) > 0) UpSeg.flip();

	// righV
	Vector3 crossBaseJointUp = cross(baseJoint.Direction, UpSeg.Direction);
	rightV = baseMaster->mPatch.getProjectedVector(crossBaseJointUp);
	rightV.normalize();

	//// debug
	//addDebugSegment(baseJoint);
	//addDebugSegment(UpSeg);
	//addDebugSegment(Geom::Segment(baseJoint.P0, baseJoint.P0 + rightV));

	// fold duration
	// this time interval stops this chain from being folded
	mFoldDuration = TIME_INTERVAL(1, 2);

	// thickness
	half_thk = 0;
	base_offset = 0;
}

void ChainGraph::fold( double t )
{
	// free all nodes
	foreach (Structure::Node* n, nodes)
		n->removeTag(FIXED_NODE_TAG);

	// fix base
	baseMaster->addTag(FIXED_NODE_TAG);

	// hinge angle
	// this works only for odd number of splits (even number of pieces)
	double half_angle = asin(1 - t);
	activeLinks.front()->hinge->angle = half_angle;
	for (int i = 1; i < activeLinks.size(); i++)
		activeLinks[i]->hinge->angle = 2 * half_angle;

	// restore configuration
	restoreConfiguration();

	// adjust the position of top master
	double h_thk = 2 * half_thk * cos(half_angle);

	double height = mMC2Trajectory.length();
	double top_dt = half_thk / height;
	double base_dt = base_offset / height;
	Geom::Segment chainUpSeg_copy = UpSeg;
	chainUpSeg_copy.cropRange01(base_dt, 1 - top_dt);
	double h_length = chainUpSeg_copy.length() / mParts.size() * (1 - t);

	int n = mParts.size();
	double top_height = half_thk + base_offset + n * h_thk + n * h_length;
	Vector3 newPos = mMC2Trajectory.P1 - top_height * mMC2Trajectory.Direction;
	topMaster->translate(newPos - topMaster->center());
}

FdGraph* ChainGraph::getKeyframe( double t )
{
	fold(t);
	return (FdGraph*)this->clone();
}

QVector<FoldOption*> ChainGraph::generateFoldOptions( int nSplits, int nUsedChunks, int nChunks )
{
	QVector<FoldOption*> options;

	double chunkSize = 1.0/double(nChunks);
	double usedSize = chunkSize * nUsedChunks;

	// position
	for (int i = 0; i <= nChunks - nUsedChunks; i++)
	{
		double position = chunkSize * i;

		// left
		QString fnid1 = QString("%1:%2_%3_L_%4").arg(mID).arg(nSplits).arg(nUsedChunks).arg(position);
		FoldOption* fn1 = new FoldOption(false, usedSize, position, nSplits, fnid1);
		options.push_back(fn1);

		// right
		QString fnid2 = QString("%1:%2_%3_R_%4").arg(mID).arg(nSplits).arg(nUsedChunks).arg(position);
		FoldOption* fn2 = new FoldOption(true, usedSize, position, nSplits, fnid2);
		options.push_back(fn2);
	}

	// fold area
	foreach (FoldOption* fn, options)
	{
		fn->region = getFoldRegion(fn);
		fn->duration = mFoldDuration;
	}

	return options;
}

void ChainGraph::createSlavePart(FoldOption* fn)
{
	// clear
	foreach (PatchNode* n, mParts) removeNode(n->mID);

	// clone original slave
	PatchNode* slave = (PatchNode*)mOrigSlave->clone();
	Structure::Graph::addNode(slave);

	// self thickness
	if (half_thk > 0) slave->setThickness(2 * half_thk);

	// horizontal modification
	int haid = slave->mBox.getAxisId(baseJoint.Direction);
	Vector3 boxAxis = slave->mBox.Axis[haid];
	double t0 = fn->position;
	double t1 = fn->position + fn->scale;
	if (dot(baseJoint.Direction, boxAxis) < 0)
		slave->mBox.scaleRange01(haid, 1-t1, 1-t0);
	else slave->mBox.scaleRange01(haid, t0, t1);

	// vertical shrinking caused by master thickness
	double height = mMC2Trajectory.length();
	double top_dt = half_thk / height;
	double base_dt = base_offset / height;
	int vaid = slave->mBox.getAxisId(UpSeg.Direction);
	if (dot(slave->mBox.Axis[vaid], UpSeg.Direction) > 0)
		slave->mBox.scaleRange01(vaid, base_dt, 1-top_dt);
	else slave->mBox.scaleRange01(vaid, top_dt, 1-base_dt);

	// update mesh and scaffold
	slave->deformMesh();
	slave->createScaffold(true);
}

void ChainGraph::sortChainParts()
{
	QMap<double, PatchNode*> distPartMap;

	Geom::Plane panel_plane = baseMaster->mPatch.getPlane();
	foreach(PatchNode* n, mParts)
	{
		double dist = panel_plane.signedDistanceTo(n->center());
		distPartMap[fabs(dist)] = n;
	}

	mParts = distPartMap.values().toVector();
}

void ChainGraph::resetHingeLinks()
{
	// remove hinge links
	rightLinks.clear(); leftLinks.clear();
	foreach (Structure::Link* link, links)
		Structure::Graph::removeLink(link);

	// shrink chain up segment for thickness
	double height = mMC2Trajectory.length();
	double top_dt = half_thk / height;
	double base_dt = base_offset / height;
	Geom::Segment upSegCopy = UpSeg;
	upSegCopy.cropRange01(base_dt, 1 - top_dt);

	// shift base joint segment for thickness
	Geom::Segment bJoint = baseJoint;
	bJoint.translate(base_offset * upSegCopy.Direction);

	// hinge links between master and slave
	Vector3 upV = UpSeg.Direction;
	Vector3 jointV = bJoint.Direction;
	Vector3 posR = bJoint.P0 + half_thk * rightV;
	Vector3 posL = bJoint.P1 - half_thk * rightV;
	Hinge* hingeR = new Hinge(mParts[0], baseMaster, 
		posR, upV,  rightV, jointV, bJoint.length());
	Hinge* hingeL = new Hinge(mParts[0], baseMaster, 
		posL, upV, -rightV, -jointV, bJoint.length());

	FdLink* linkR = new FdLink(mParts[0], baseMaster, hingeR);
	FdLink* linkL = new FdLink(mParts[0], baseMaster, hingeL);
	Graph::addLink(linkR);	
	Graph::addLink(linkL);
	rightLinks << linkR; 
	leftLinks << linkL;

	// hinge links between two parts in the chain
	// there are no hinges for top master
	double step = 2.0 / mParts.size();
	for (int i = 1; i < mParts.size(); i++)
	{
		PatchNode* part1 = mParts[i];
		PatchNode* part2 = mParts[i-1];

		Vector3 pos = upSegCopy.getPosition(-1 + step * i);
		Vector3 deltaV =  pos - upSegCopy.P0;
		Geom::Segment joint = bJoint.translated(deltaV);

		Vector3 upV = UpSeg.Direction;
		Vector3 jointV = joint.Direction;
		Vector3 posR = joint.P0 + half_thk * rightV;
		Vector3 posL = joint.P1 - half_thk * rightV;
		Hinge* hingeR = new Hinge(part1, part2, 
			posR, upV, -upV, jointV, joint.length());
		Hinge* hingeL = new Hinge(part1, part2, 
			posL, upV, -upV, -jointV, joint.length());

		FdLink* linkR = new FdLink(part1, part2, hingeR);
		FdLink* linkL = new FdLink(part1, part2, hingeL);
		Graph::addLink(linkR);	
		Graph::addLink(linkL);
		rightLinks << linkR; 
		leftLinks << linkL;
	}
}

void ChainGraph::setActiveLinks( FoldOption* fn )
{
	// clear
	activeLinks.clear();

	// set hinge for each joint
	for (int i = 0; i < rightLinks.size(); i++)
	{
		// inactive both hinges
		rightLinks[i]->removeTag(ACTIVE_HINGE_TAG);
		leftLinks[i]->removeTag(ACTIVE_HINGE_TAG);

		// active link
		FdLink* activeLink;
		if (i % 2 == 0)
			activeLink = (fn->rightSide) ? rightLinks[i] : leftLinks[i];
		else
			activeLink = (fn->rightSide) ? leftLinks[i] : rightLinks[i];

		activeLink->addTag(ACTIVE_HINGE_TAG);
		activeLinks << activeLink;
	}
}

void ChainGraph::applyFoldOption( FoldOption* fn)
{
	// delete chain if fold option is null
	if (fn->hasTag(DELETE_FOLD_OPTION)) {
		addTag(DELETED_TAG);
		std::cout << "Chain: " << mID.toStdString() << " is deleted.\n";
		return;
	}
	
	// clear tag
	removeTag(DELETED_TAG); 

	createSlavePart(fn);

	mParts.clear();
	QVector<Geom::Plane> cutPlanes = generateCutPlanes(fn->nSplits);
	QVector<FdNode*> splitNodes = FdGraph::split(mOrigSlave->mID, cutPlanes);
	foreach (FdNode* n, splitNodes) mParts << (PatchNode*)n;

	sortChainParts();
	resetHingeLinks();

	setActiveLinks(fn);
}

void ChainGraph::setFoldDuration( double t0, double t1 )
{
	if (t0 > t1) std::swap(t0, t1);
	t0 += ZERO_TOLERANCE_LOW;
	t1 -= ZERO_TOLERANCE_LOW; 

	mFoldDuration = TIME_INTERVAL(t0, t1);
}

Geom::Rectangle ChainGraph::getFoldRegion( FoldOption* fn )
{
	// axis and rightV
	Vector3 rV = rightV;
	Geom::Segment seg1 = baseJoint;
	if (!fn->rightSide) rV *= -1;

	// shrink along axisSeg
	double t0 = fn->position;
	double t1 = t0 + fn->scale;
	seg1.cropRange01(t0, t1);
	Geom::Segment seg2 = seg1;

	// shift the axis along rightV
	double width = UpSeg.length() / (fn->nSplits + 1);
	seg2.translate(width * rV);

	// shrink epsilon
	Geom::Rectangle rect(QVector<Vector3>() 
		<< seg1.P0	<< seg1.P1
		<< seg2.P1  << seg2.P0 );
	rect.scale(1 - ZERO_TOLERANCE_LOW);
	return rect;
}

Geom::Rectangle ChainGraph::getMaxFoldRegion( bool right )
{
	FoldOption fn(right, 1.0, 0.0, 1, "");
	return getFoldRegion(&fn);
}

QVector<Geom::Plane> ChainGraph::generateCutPlanes( int nbSplit )
{
	// plane of master0
	Geom::Plane base_plane = baseMaster->mPatch.getPlane();
	if (base_plane.whichSide(mOrigSlave->center()) < 0) base_plane.flip();

	// thickness
	Vector3 dv = base_offset * base_plane.Normal;
	base_plane.translate(dv);

	// step to shift up
	double height = mMC2Trajectory.length() - base_offset - half_thk;
	double step = height / (nbSplit + 1);

	// create planes
	QVector<Geom::Plane> cutPlanes;
	for (int i = 0; i < nbSplit; i++)
	{
		Vector3 deltaV = (i+1) * step * base_plane.Normal;
		cutPlanes << base_plane.translated(deltaV);
	}

	// to-do: is N is even, cutting will be different

	return cutPlanes;
}
