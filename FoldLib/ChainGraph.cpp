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
		
		origSlave = new PatchNode(slaveRod, slavePatchNorm);
	}
	else
		origSlave = (PatchNode*)slave->clone();

	chainParts << (PatchNode*)origSlave->clone();
	Structure::Graph::addNode(chainParts[0]);

	// masters
	baseMaster = (PatchNode*)base->clone();
	topMaster = (PatchNode*)top->clone();
	Graph::addNode(baseMaster);
	Graph::addNode(topMaster);

	// joints
	baseJoint = detectJointSegment(origSlave, baseMaster);
	topJoint = detectJointSegment(origSlave, topMaster);
	if (dot(topJoint.Direction, baseJoint.Direction) < 0) topJoint.flip();

	// slaveSeg
	Vector3 bJointP0 = baseJoint.P0;
	QVector<Geom::Segment> edges = origSlave->mPatch.getPerpEdges(baseJoint.Direction);
	slaveSeg = edges[0].contains(bJointP0) ? edges[0] : edges[1];
	if (slaveSeg.getProjCoordinates(bJointP0) > 0) slaveSeg.flip();

	// rightSeg
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Segment topJointProj = base_rect.getProjection(topJoint);
	rightSeg = Geom::Segment(topJointProj.P0, baseJoint.P0);
	if (rightSeg.length() > ZERO_TOLERANCE_LOW)
		rightSegV = rightSeg.Direction;
	else{
		Vector3 crossJointUp = cross(baseJoint.Direction, slaveSeg.Direction);
		rightSegV = base_rect.getProjectedVector(crossJointUp);
	}

	// upV x rightV = jointV
	Vector3 crossUpRight = cross(slaveSeg.Direction, rightSeg.Direction);
	if (dot(crossUpRight, baseJoint.Direction) < 0){
		baseJoint.flip(); topJoint.flip();
	}

	// topTraj
	Vector3 topCenterProj = base_rect.getProjection(top->center());
	topTraj = Geom::Segment(topCenterProj, top->center());

	// fold duration
	duration = INTERVAL(1, 2);

	// thickness
	halfThk = 0;
	baseOffset = 0;

	// side
	foldToRight = true;

	//// debug
	//addDebugSegment(baseJoint);
	//addDebugSegment(UpSeg);
	//addDebugSegment(Geom::Segment(baseJoint.P0, baseJoint.P0 + rightV));
}

void ChainGraph::fold( double t )
{
	// free all nodes
	foreach (Structure::Node* n, nodes)
		n->removeTag(FIXED_NODE_TAG);

	// fix base
	baseMaster->addTag(FIXED_NODE_TAG);

	// hinge angles
	double d = rightSeg.length() * getShunkScale();
	double sl = getShrunkSlaveSeg().length();
	int n = chainParts.size();
	double a = (sl - d) / n;
	double b = d + a;
	double alpha = acos(d / sl) * (1 - t);
	double cos_beta = (b * cos(alpha) - d) / a;
	double beta = acos(cos_beta);
	if (foldToRight) std::swap(alpha, beta);

	activeLinks[0]->hinge->angle = alpha;
	activeLinks[1]->hinge->angle = alpha + beta;
	for (int i = 2; i < activeLinks.size(); i++)
		activeLinks[i]->hinge->angle = 2 * beta;

	// restore configuration
	restoreConfiguration();

	// adjust the position of top master
	double c = 2 * halfThk;
	double ha = a * sin(beta) + c * cos(beta);
	double hb = b * sin(alpha) + c * cos(alpha);
	double topH = (n - 1) * ha + hb + halfThk + baseOffset;
	Vector3 topPos = topTraj.P0 + topH * topTraj.Direction;
	topMaster->translate(topPos - topMaster->center());
}

FdGraph* ChainGraph::getKeyframe( double t )
{
	fold(t);

	//FdGraph* keyframe = new FdGraph(mID);
	//foreach (FdNode* n, getFdNodes())
	//	keyframe->Structure::Graph::addNode(n->clone());

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
		fn->duration = duration;
	}

	return options;
}

void ChainGraph::resetChainParts(FoldOption* fn)
{
	// clear
	foreach (PatchNode* n, chainParts) removeNode(n->mID);
	chainParts.clear();

	// clone original slave
	PatchNode* slave = (PatchNode*)origSlave->clone();
	Structure::Graph::addNode(slave);

	// self thickness
	if (halfThk > 0) slave->setThickness(2 * halfThk);

	// horizontal modification
	int aid_h = slave->mBox.getAxisId(baseJoint.Direction);
	Vector3 boxAxis = slave->mBox.Axis[aid_h];
	double t0 = fn->position;
	double t1 = fn->position + fn->scale;
	if (dot(baseJoint.Direction, boxAxis) > 0)
		slave->mBox.scaleRange01(aid_h, t0, t1);
	else slave->mBox.scaleRange01(aid_h, 1-t1, 1-t0);

	// vertical shrinking caused by master thickness
	Interval it = getShunkInterval();
	int aid_v = slave->mBox.getAxisId(slaveSeg.Direction);
	if (dot(slave->mBox.Axis[aid_v], slaveSeg.Direction) > 0)
		slave->mBox.scaleRange01(aid_v, it.first, it.second);
	else slave->mBox.scaleRange01(aid_v, 1-it.second, 1-it.first);

	// update mesh and scaffold
	slave->deformMesh();
	slave->createScaffold(true);

	// cut into pieces
	QVector<Geom::Plane> cutPlanes = generateCutPlanes(fn);
	QVector<FdNode*> splitNodes = FdGraph::split(origSlave->mID, cutPlanes);
	foreach (FdNode* n, splitNodes) chainParts << (PatchNode*)n; 

	// sort them
	QMap<double, PatchNode*> distPartMap;
	Geom::Plane base_plane = baseMaster->mPatch.getPlane();
	foreach(PatchNode* n, chainParts)
	{
		double dist = base_plane.signedDistanceTo(n->center());
		distPartMap[fabs(dist)] = n;
	}
	chainParts = distPartMap.values().toVector();
}

void ChainGraph::resetHingeLinks(FoldOption* fn)
{
	// remove hinge links
	rightLinks.clear(); leftLinks.clear();
	foreach (Structure::Link* link, links)
		Structure::Graph::removeLink(link);

	// shift base joint segment for thickness
	Geom::Segment bJoint = baseJoint;
	bJoint.translate(baseOffset * slaveSeg.Direction);

	// hinge links between master and slave
	Vector3 upV = slaveSeg.Direction;
	Vector3 jointV = bJoint.Direction;
	Vector3 rV = rightSegV;
	Vector3 posR = bJoint.P0 + halfThk * rV;
	Vector3 posL = bJoint.P1 - halfThk * rV;
	Hinge* hingeR = new Hinge(chainParts[0], baseMaster, 
		posR, upV,  rV, jointV, bJoint.length());
	Hinge* hingeL = new Hinge(chainParts[0], baseMaster, 
		posL, upV, -rV, -jointV, bJoint.length());

	FdLink* linkR = new FdLink(chainParts[0], baseMaster, hingeR);
	FdLink* linkL = new FdLink(chainParts[0], baseMaster, hingeL);
	Graph::addLink(linkR);	
	Graph::addLink(linkL);
	rightLinks << linkR; 
	leftLinks << linkL;

	// hinge links between two parts in the chain
	double l = getShrunkSlaveSeg().length();
	double d = getShunkScale() * rightSeg.length();
	double h = getShrunkTopTraj().length();
	double step = (l - d) / (fn->nSplits + 1);
	if (!fn->rightSide){
		bJoint.translate(d * slaveSeg.Direction);
	}

	for (int i = 1; i < chainParts.size(); i++)
	{
		bJoint.translate(step * slaveSeg.Direction);

		PatchNode* part1 = chainParts[i];
		PatchNode* part2 = chainParts[i-1];
		Vector3 upV = slaveSeg.Direction;
		Vector3 jointV = bJoint.Direction;
		Vector3 posR = bJoint.P0 + halfThk * rV;
		Vector3 posL = bJoint.P1 - halfThk * rV;
		Hinge* hingeR = new Hinge(part1, part2, 
			posR, upV, -upV, jointV, bJoint.length());
		Hinge* hingeL = new Hinge(part1, part2, 
			posL, upV, -upV, -jointV, bJoint.length());

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

	foldToRight = fn->rightSide;
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

	resetChainParts(fn);
	resetHingeLinks(fn);
	setActiveLinks(fn);
}

void ChainGraph::setFoldDuration( double t0, double t1 )
{
	if (t0 > t1) std::swap(t0, t1);
	t0 += ZERO_TOLERANCE_LOW;
	t1 -= ZERO_TOLERANCE_LOW; 

	duration = INTERVAL(t0, t1);
}

Geom::Rectangle ChainGraph::getFoldRegion( FoldOption* fn )
{
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Segment topJointProj = base_rect.getProjection(topJoint);

	double d = rightSeg.length();
	double l = slaveSeg.length();
	double offset = (l - d) / (fn->nSplits + 1);

	Geom::Segment rightSeg, leftSeg;
	if (fn->rightSide)
	{
		leftSeg = topJointProj;
		rightSeg = baseJoint.translated(offset * rightSegV);
	}
	else
	{
		leftSeg = topJointProj.translated(-offset * rightSegV);
		rightSeg = baseJoint;
	}

	// shrink along jointV
	double t0 = fn->position;
	double t1 = t0 + fn->scale;
	leftSeg.cropRange01(t0, t1);
	rightSeg.cropRange01(t0, t1);

	// fold region
	Geom::Rectangle region(QVector<Vector3>() 
		<< leftSeg.P0	<< leftSeg.P1
		<< rightSeg.P1  << rightSeg.P0 );

	addDebugSegment(leftSeg);
	addDebugSegment(rightSeg);

	return region;
}

Geom::Rectangle ChainGraph::getMaxFoldRegion( bool right )
{
	FoldOption fn(right, 1.0, 0.0, 1, "");
	return getFoldRegion(&fn);
}


//     left					right
//	   ---						|
//		|step					|
//	   ---						|
//		|step					|d
//	  -----	plane0				|
//		|						|
//		|					  -----
//		|						|step
//		|d					   ---
//		|						|step
//		|					   --- plane0

QVector<Geom::Plane> ChainGraph::generateCutPlanes( FoldOption* fn )
{
	// base plane
	Geom::Plane plane0 = baseMaster->mPatch.getPlane();
	plane0.translate(baseOffset * plane0.Normal);

	// scales
	double l = getShrunkSlaveSeg().length();
	double d = getShunkScale() * rightSeg.length();
	double h = getShrunkTopTraj().length();
	double step = (l - d) / (fn->nSplits + 1);
	double vstep = step * h / l;
	if (!fn->rightSide){
		double offset = d * h / l;
		plane0.translate(offset * plane0.Normal);
	}

	// cut planes
	QVector<Geom::Plane> cutPlanes;
	for (int i = 1; i <= fn->nSplits; i++)
	{
		Vector3 offsetV = i * vstep * plane0.Normal;
		cutPlanes << plane0.translated(offsetV);
	}

	// debug
	appendToVectorProperty<Geom::Plane>(DEBUG_PLANES, cutPlanes);

	return cutPlanes;
}

Geom::Segment ChainGraph::getShrunkSlaveSeg()
{
	Geom::Segment seg = slaveSeg;
	Interval it = getShunkInterval();
	seg.cropRange01(it.first, it.second);

	return seg;
}

double ChainGraph::getShunkScale()
{
	Interval it = getShunkInterval();
	return 1 - it.first - (1 - it.second);
}

Geom::Segment ChainGraph::getShrunkTopTraj()
{
	Geom::Segment seg = topTraj;
	Interval it = getShunkInterval();
	seg.cropRange01(it.first, it.second);

	return seg;
}

Interval ChainGraph::getShunkInterval()
{
	double height = topTraj.length();
	double top_dt = halfThk / height;
	double base_dt = baseOffset / height;

	return INTERVAL(base_dt, 1 - top_dt);
}
