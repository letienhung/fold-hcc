#include "ChainScaff.h"
#include "FdUtility.h"
#include "RodNode.h"
#include "Numeric.h"
#include "ParSingleton.h"
#include "DistSegRect.h"

ChainScaff::ChainScaff( ScaffNode* slave, PatchNode* base, PatchNode* top)
	: Scaffold(slave->mID)
{
	// masters, slave, and thickness
	setupMasters(base, top);
	setupSlave(slave);
	setupThickness();

	// basic orientations
	computeBasicOrientations();
	computeRightDirection();
	showBasicOrientations();

	// fold parameters
	duration.set(1, 2);
	foldToRight = true;
	isDeleted = false;
	importance = 0;
}

void ChainScaff::setupMasters(PatchNode* base, PatchNode* top)
{
	baseMaster = (PatchNode*)base->clone();
	topMaster = (PatchNode*)top->clone();
	Graph::addNode(baseMaster);
	Graph::addNode(topMaster);
}

void ChainScaff::setupSlave(ScaffNode* slave)
{
	// clone
	if (slave->mType == ScaffNode::ROD)
	{
		// convert slave into patch if need
		RodNode* slaveRod = (RodNode*)slave;
		Vector3 rodV = slaveRod->mRod.Direction;
		Vector3 baseV = baseMaster->mPatch.Normal;
		Vector3 crossRodVBaseV = cross(rodV, baseV);
		Vector3 slavePatchNorm;
		if (crossRodVBaseV.norm() < 0.1)
			slavePatchNorm = baseMaster->mPatch.Axis[0];
		else slavePatchNorm = cross(crossRodVBaseV, rodV);
		slavePatchNorm.normalize();

		origSlave = new PatchNode(slaveRod, slavePatchNorm);
	}
	else{
		origSlave = (PatchNode*)slave->clone();
	}

	// deform to attach masters perfectly
	origSlave->deformToAttach(baseMaster->getSurfacePlane(true));
	if (!topMaster->hasTag(EDGE_VIRTUAL_TAG)) // skip top virtual for T-chain
		origSlave->deformToAttach(topMaster->getSurfacePlane(false));

	// save parts
	chainParts << (PatchNode*)origSlave->clone();
	Structure::Graph::addNode(chainParts[0]);
}

void ChainScaff::setupThickness()
{
	topHThk = topMaster->getThickness() / 2;
	baseHThk = baseMaster->getThickness() / 2;
	slaveHThk = origSlave->getThickness() / 2;
}


void ChainScaff::computeBasicOrientations()
{
	// the top and base patch edges 
	QVector<Geom::Segment> perpEdges = origSlave->mPatch.getPerpEdges(baseMaster->mPatch.Normal);
	Geom::DistSegRect dsr0(perpEdges[0], baseMaster->mPatch);
	Geom::DistSegRect dsr1(perpEdges[1], baseMaster->mPatch);
	Geom::Segment topEdge = perpEdges[0];
	Geom::Segment baseEdge = perpEdges[1];
	if (dsr0.get() < dsr1.get()) {
		baseEdge = perpEdges[0]; topEdge = perpEdges[1];
	}
	if (dot(topEdge.Direction, baseEdge.Direction) < 0) topEdge.flip();

	// the up right segment
	Geom::Rectangle baseSurface = baseMaster->getSurfaceRect(true);
	Geom::Segment topEdgeProj = baseSurface.getProjection(topEdge);
	upSeg.set(topEdgeProj.Center, topEdge.Center);
	double topTrimRatio = slaveHThk / upSeg.length();
	double baseTrimRatio = slaveHThk / upSeg.length();
	if (topMaster->hasTag(EDGE_VIRTUAL_TAG)) topTrimRatio = 0;
	upSeg.cropRange01(baseTrimRatio, 1 - topTrimRatio);

	// slaveSeg : bottom to up
	slaveSeg.set(baseEdge.Center, topEdge.Center);
	slaveSeg.cropRange01(baseTrimRatio, 1 - topTrimRatio);

	// the joints
	baseJoint = baseEdge.translated(slaveSeg.P0 - baseEdge.Center);
	topJoint = topEdge.translated(slaveSeg.P1 - topEdge.Center);
}


void ChainScaff::computeRightDirection()
{
	// right segment and right direction
	// baseJoint, slaveSeg and rightDirect are right-handed
	Geom::Rectangle baseSurface = baseMaster->getSurfaceRect(true);
	Geom::Rectangle baseSurfaceVirtual = baseSurface;
	baseSurfaceVirtual.translate(slaveHThk * upSeg.Direction);
	Vector3 topCentreProj = baseSurfaceVirtual.getProjection(topJoint.Center);
	rightSeg.set(topCentreProj, baseJoint.Center);
	if (rightSeg.length() / slaveSeg.length() < 0.1) // around 5 degrees
	{
		rightDirect = cross(baseJoint.Direction, slaveSeg.Direction);
		rightDirect = baseSurface.getProjectedVector(rightDirect);
		rightDirect.normalize();
	}
	else
	{
		rightDirect = rightSeg.Direction;
		Vector3 crossSlaveRight = cross(slaveSeg.Direction, rightDirect);
		if (dot(crossSlaveRight, baseJoint.Direction) < 0){
			baseJoint.flip(); topJoint.flip();
		}
	}

	// flip slave patch so that its norm is to the right
	if (dot(origSlave->mPatch.Normal, rightDirect) < 0)
		origSlave->mPatch.flipNormal();
}

ChainScaff::~ChainScaff()
{
	delete origSlave;
}

// the keyframe cannot be nullptr
Scaffold* ChainScaff::getKeyframe( double t, bool useThk )
{
	Scaffold* keyframe = nullptr;

	// deleted chain only has the base master
	if (isDeleted)
	{
		keyframe = new Scaffold();
		keyframe->Structure::Graph::addNode(baseMaster->clone());
		return keyframe;
	}
	// otherwise fold the chain and clone
	else
	{
		// fold w\o thickness
		fold(t);
		keyframe = (Scaffold*)this->clone();

		//// thickness
		//if (useThk) addThickness(keyframe, t);
	}

	return keyframe;
}

void ChainScaff::setFoldDuration( double t0, double t1 )
{
	if (t0 > t1) std::swap(t0, t1);
	t0 += ZERO_TOLERANCE_LOW;
	t1 -= ZERO_TOLERANCE_LOW; 

	duration.set(t0, t1);
}

FoldOption* ChainScaff::genFoldOption(QString id, bool isRight, double width, double startPos, int nSplits)
{
	FoldOption* fn = new FoldOption(id, isRight, width, startPos, nSplits);
	fn->region = getFoldRegion(fn);
	fn->duration = duration;

	return fn;
}

QVector<FoldOption*> ChainScaff::genRegularFoldOptions( int nSplits, int nChunks )
{
	int maxNbChunks = ParSingleton::instance()->maxNbChunks;
	double chunkWidth = 1.0/double(maxNbChunks);
	double usedWidth = chunkWidth * nChunks;

	QVector<FoldOption*> options;
	for (int i = 0; i <= maxNbChunks - nChunks; i++)
	{
		double startPos = chunkWidth * i;

		// right
		QString fnid1 = QString("%1:%2_%3_L_%4").arg(mID).arg(nSplits).arg(nChunks).arg(startPos);
		options << genFoldOption(fnid1, true, usedWidth, startPos, nSplits);

		// left
		QString fnid2 = QString("%1:%2_%3_R_%4").arg(mID).arg(nSplits).arg(nChunks).arg(startPos);
		options << genFoldOption(fnid2, false, usedWidth, startPos, nSplits);
	}

	return options;
}

FoldOption* ChainScaff::genDeleteFoldOption()
{
	// keep the number of splits for computing the cost
	QString fnid = mID + "_delete";
	int nS = ParSingleton::instance()->maxNbSplits;
	FoldOption* delete_fn = new FoldOption(fnid, true, 0, 0, nS);
	return delete_fn;
}


void ChainScaff::resetChainParts(FoldOption* fn)
{
	// clear
	for (PatchNode* n : chainParts) removeNode(n->mID);
	chainParts.clear();

	if(!origSlave) return;

	// clone original slave
	PatchNode* slave = (PatchNode*)origSlave->clone();
	Structure::Graph::addNode(slave);

	// shrink along joint direction
	int aid_h = slave->mBox.getAxisId(baseJoint.Direction);
	Vector3 boxAxis = slave->mBox.Axis[aid_h];
	double t0 = fn->position;
	double t1 = fn->position + fn->scale;
	if (dot(baseJoint.Direction, boxAxis) > 0)
		slave->mBox.scaleRange01(aid_h, t0, t1);
	else slave->mBox.scaleRange01(aid_h, 1-t1, 1-t0);

	// update mesh and scaffold
	slave->deformMesh();
	slave->createScaffold(true);

	// cut into pieces
	QVector<Geom::Plane> cutPlanes = generateCutPlanes(fn);
	QVector<ScaffNode*> splitNodes = Scaffold::split(origSlave->mID, cutPlanes);
	for (ScaffNode* n : splitNodes) chainParts << (PatchNode*)n;

	// sort them
	QMap<double, PatchNode*> distPartMap;
	Geom::Plane base_plane = baseMaster->mPatch.getPlane();
	for (PatchNode* n : chainParts)
	{
		double dist = base_plane.signedDistanceTo(n->center());
		distPartMap[fabs(dist)] = n;
	}
	chainParts = distPartMap.values().toVector();

	// debug
	visDebug.addPlanes(cutPlanes, Qt::red);
}

void ChainScaff::resetHingeLinks(FoldOption* fn)
{
	// remove hinge links
	qDeleteAll(links);	links.clear();
	rightLinks.clear(); leftLinks.clear();

	// basic orientations
	Geom::Segment bJoint = baseJoint;
	double jointLen = bJoint.length();
	Vector3 slaveV = slaveSeg.Direction;
	Vector3 jointV = bJoint.Direction;

	// ***hinge links between base and slave
	Hinge* hingeR = new Hinge(chainParts[0], baseMaster, 
		bJoint.P0, slaveV, rightDirect, jointV, jointLen);
	Hinge* hingeL = new Hinge(chainParts[0], baseMaster, 
		bJoint.P1, slaveV, -rightDirect, -jointV, jointLen);
	ScaffLink* linkR = new ScaffLink(chainParts[0], baseMaster, hingeR);
	ScaffLink* linkL = new ScaffLink(chainParts[0], baseMaster, hingeL);
	Graph::addLink(linkR);	
	Graph::addLink(linkL);
	rightLinks << linkR; 
	leftLinks << linkL;

	// ***hinge links between two parts in the chain
	double l = slaveSeg.length();
	double d = rightSeg.length();
	double step = (l - d) / (fn->nSplits + 1);
	if (!fn->rightSide){
		bJoint.translate(d * slaveV);
	}

	// shift along the salve normal (the right)
	Vector3 jointOffsetR;// = slaveHThk * origSlave->mPatch.Normal;
	for (int i = 1; i < chainParts.size(); i++)
	{
		PatchNode* part1 = chainParts[i];
		PatchNode* part2 = chainParts[i-1];

		bJoint.translate(step * slaveV);
		Hinge* hingeR = new Hinge(part1, part2, 
			bJoint.P0 + jointOffsetR, slaveV, -slaveV, jointV, jointLen);
		Hinge* hingeL = new Hinge(part1, part2, 
			bJoint.P1 - jointOffsetR, slaveV, -slaveV, -jointV, jointLen);

		ScaffLink* linkR = new ScaffLink(part1, part2, hingeR);
		ScaffLink* linkL = new ScaffLink(part1, part2, hingeL);
		Graph::addLink(linkR);	
		Graph::addLink(linkL);
		rightLinks << linkR; 
		leftLinks << linkL;
	}
}

void ChainScaff::activateLinks( FoldOption* fn )
{
	// clear
	activeLinks.clear();

	// set hinge for each joint
	for (int i = 0; i < rightLinks.size(); i++)
	{
		// inactive both hinges
		rightLinks[i]->removeTag(ACTIVE_LINK_TAG);
		leftLinks[i]->removeTag(ACTIVE_LINK_TAG);

		// active link
		ScaffLink* activeLink;
		if (i % 2 == 0)
			activeLink = (fn->rightSide) ? rightLinks[i] : leftLinks[i];
		else
			activeLink = (fn->rightSide) ? leftLinks[i] : rightLinks[i];

		activeLink->addTag(ACTIVE_LINK_TAG);
		activeLinks << activeLink;
	}

	// which side
	foldToRight = fn->rightSide;
}

void ChainScaff::applyFoldOption( FoldOption* fn)
{
	// delete chain if fold option is null
	if (fn->scale == 0) {
		isDeleted = true;
		std::cout << "Chain: " << mID.toStdString() << " is deleted.\n";
		return;
	}
	
	// clear tag
	isDeleted = false; 

	// reset chains and hinges
	resetChainParts(fn);
	resetHingeLinks(fn);
	activateLinks(fn);
}


//void ChainScaff::addThickness(Scaffold* keyframe, double t)
//{
//	if (!baseMaster) return;
//
//	// get parts in key frame
//	QVector<PatchNode*> keyParts;
//	keyParts << (PatchNode*)keyframe->getNode(baseMaster->mID);
//	for (PatchNode* cpart : chainParts) keyParts << (PatchNode*)keyframe->getNode(cpart->mID);
//	keyParts << (PatchNode*)keyframe->getNode(topMaster->mID);
//
//	// move parts up for thickness
//	for (int i = 1; i < keyParts.size(); i++)
//	{
//		// compute offset caused by thickness between part_i and part_i_1
//		Vector3 offset(0, 0, 0);
//		Vector3 v1(0, 0, 0), v2(0, 0, 0);
//		if (i == 1)
//		{
//			// base
//			Geom::Rectangle rect1 = keyParts[i]->mPatch;
//			v2 = keyParts[0]->mPatch.Normal; //up
//
//			double dotNN = dot(rect1.Normal, v2);
//			if (dotNN > ZERO_TOLERANCE_LOW) v1 = rect1.Normal;
//			else if (dotNN < -ZERO_TOLERANCE_LOW) v1 = -rect1.Normal;
//
//			offset = halfThk * v1 + baseOffset * v2;
//		}
//		else if (i == keyParts.size() - 1)
//		{
//			// top
//			v1 = keyParts[0]->mPatch.Normal; //up
//			Geom::Rectangle rect2 = keyParts[i - 1]->mPatch;
//
//			double dotNN = dot(v1, rect2.Normal);
//			if (dotNN > ZERO_TOLERANCE_LOW) v2 = -rect2.Normal;
//			else if (dotNN < -ZERO_TOLERANCE_LOW) v2 = rect2.Normal;
//
//			offset = halfThk * (v1 - v2);
//		}
//		else
//		{
//			// chain parts
//			Geom::Rectangle rect1 = keyParts[i]->mPatch;
//			Geom::Rectangle rect2 = keyParts[i - 1]->mPatch;
//
//			// flattened
//			if (1 - t < ZERO_TOLERANCE_LOW)
//			{
//				v1 = baseMaster->mPatch.Normal;
//				v2 = -v1;
//			}
//			else
//			{
//				int side2to1 = rect1.whichSide(rect2.Center);
//				if (side2to1 == 1) v1 = -rect1.Normal;
//				else if (side2to1 == -1) v1 = rect1.Normal;
//
//				int side1to2 = rect2.whichSide(rect1.Center);
//				if (side1to2 == 1) v2 = -rect2.Normal;
//				else if (side1to2 == -1) v2 = rect2.Normal;
//			}
//
//			offset = halfThk * (v1 - v2);
//		}
//
//		// translate parts above
//		for (int j = i; j < keyParts.size(); j++)
//			keyParts[j]->translate(offset);
//	}
//
//	//// set thickness to chain parts
//	//for (int i = 1; i < keyParts.size() - 1; i++)
//	//	keyParts[i]->setThickness(2 * halfThk);
//}

double ChainScaff::getSlaveArea()
{
	return origSlave->mPatch.area();
}

void ChainScaff::showBasicOrientations()
{
	// debug
	visDebug.addSegment(baseJoint, Qt::blue);
	visDebug.addSegment(topJoint, Qt::blue);
	visDebug.addSegment(slaveSeg, Qt::black);
	visDebug.addSegment(upSeg, Qt::green);
	visDebug.addSegment(rightSeg, Qt::red);
	Geom::Segment rightV(baseJoint.P0, baseJoint.P0 + baseJoint.length() * rightDirect);
	visDebug.addSegment(rightV, Qt::red);
}
