#include "ChainScaff.h"
#include "FdUtility.h"
#include "RodNode.h"
#include "Numeric.h"

ChainScaff::ChainScaff( ScaffNode* slave, PatchNode* base, PatchNode* top)
	: Scaffold(slave->mID)
{
	// slave
	if (slave->mType == ScaffNode::ROD)
	{
		if(!baseMaster) return;

		// convert slave into patch if need
		RodNode* slaveRod = (RodNode*)slave;
		Vector3 rodV = slaveRod->mRod.Direction;
		Vector3 baseV = base->mPatch.Normal;
		Vector3 crossRodVBaseV = cross(rodV, baseV);
		Vector3 slavePatchNorm;
		if (crossRodVBaseV.norm() < 0.1)	
			slavePatchNorm = baseMaster->mPatch.Axis[0];
		else slavePatchNorm = cross(crossRodVBaseV, rodV);
		slavePatchNorm.normalize();
		
		origSlave = new PatchNode(slaveRod, slavePatchNorm);
	}else
	{
		origSlave = (PatchNode*)slave->clone();
	}
	chainParts << (PatchNode*)origSlave->clone();
	Structure::Graph::addNode(chainParts[0]);

	// masters
	baseMaster = (PatchNode*)base->clone();
	topMaster = (PatchNode*)top->clone();
	Graph::addNode(baseMaster);
	Graph::addNode(topMaster);

	// set up orientations
	computeOrientations();

	// fold duration
	duration.set(1, 2);

	// thickness
	halfThk = 0;
	baseOffset = 0;

	// side
	foldToRight = true;

	// delete
	isDeleted = false;

	//// debug
	//addDebugSegment(baseJoint);
	//addDebugSegment(slaveSeg);
	//addDebugSegment(rightSeg);
	//addDebugSegment(Geom::Segment(baseJoint.P0, baseJoint.P0 + rightSegV));
}

ChainScaff::~ChainScaff()
{
	delete origSlave;
}

void ChainScaff::computeOrientations()
{
	// joints
	baseJoint = detectJointSegment(origSlave, baseMaster);
	topJoint = detectJointSegment(origSlave, topMaster);
	if (dot(topJoint.Direction, baseJoint.Direction) < 0) topJoint.flip();

	// slaveSeg
	Vector3 bJointP0 = baseJoint.P0;
	Geom::Rectangle slave_rect = origSlave->mPatch;
	slaveSeg = slave_rect.getSkeleton(slave_rect.getPerpAxis(baseJoint.Direction));
	if (slaveSeg.getProjCoordinates(bJointP0) > 0) slaveSeg.flip();

	// rightSeg and direction
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Segment topJointProj = base_rect.getProjection(topJoint);
	rightSeg = Geom::Segment(topJointProj.P0, baseJoint.P0);
	if (rightSeg.length() / slaveSeg.length() < 0.01){
		rightSegV = cross(baseJoint.Direction, slaveSeg.Direction);
		rightSegV = base_rect.getProjectedVector(rightSegV);
	}else{
		rightSegV = rightSeg.Direction;
	}
	rightSegV.normalize();

	// flip slave patch so that its norm is to the right
	if (dot(origSlave->mPatch.Normal, rightSegV) < 0)
		origSlave->mPatch.flipNormal();

	// upV x rightV = jointV
	Vector3 crossUpRight = cross(slaveSeg.Direction, rightSegV);
	if (dot(crossUpRight, baseJoint.Direction) < 0){
		baseJoint.flip(); topJoint.flip();
	}

	// topTraj
	Vector3 topCenterProj = base_rect.getProjection(topMaster->center());
	topTraj = Geom::Segment(topCenterProj, topMaster->center());
}

Scaffold* ChainScaff::getKeyframe( double t, bool useThk )
{
	// fold w\o thickness
	fold(t);

	// key frame
	Scaffold* keyframe = (Scaffold*)this->clone();

	// thickness
	if (useThk) addThickness(keyframe, t);

	return keyframe;
}

void ChainScaff::setFoldDuration( double t0, double t1 )
{
	if (t0 > t1) std::swap(t0, t1);
	t0 += ZERO_TOLERANCE_LOW;
	t1 -= ZERO_TOLERANCE_LOW; 

	duration.set(t0, t1);
}

QVector<FoldOption*> ChainScaff::genFoldOptionWithDiffPositions( int nSplits, int nChunks, int maxNbChunks )
{
	QVector<FoldOption*> options;

	double chunkWidth = 1.0/double(maxNbChunks);
	double usedWidth = chunkWidth * nChunks;

	// start position
	for (int i = 0; i <= maxNbChunks - nChunks; i++)
	{
		double startPos = chunkWidth * i;

		// left
		QString fnid1 = QString("%1:%2_%3_L_%4").arg(mID).arg(nSplits).arg(nChunks).arg(startPos);
		FoldOption* fn1 = new FoldOption(fnid1, false, usedWidth, startPos, nSplits);
		options.push_back(fn1);

		// right
		QString fnid2 = QString("%1:%2_%3_R_%4").arg(mID).arg(nSplits).arg(nChunks).arg(startPos);
		FoldOption* fn2 = new FoldOption(fnid2, true, usedWidth, startPos, nSplits);
		options.push_back(fn2);
	}

	// fold region and duration
	foreach(FoldOption* fn, options)
	{
		fn->region = getFoldRegion(fn);
		fn->duration = duration;
	}

	return options;
}

FoldOption* ChainScaff::genDeleteFoldOption( int nSplits )
{
	// keep the number of slipts for computing the cost
	QString fnid = mID + "_delete";
	FoldOption* delete_fn = new FoldOption(fnid, true, 0, 0, nSplits);
	return delete_fn;
}


void ChainScaff::resetChainParts(FoldOption* fn)
{
	// clear
	foreach (PatchNode* n, chainParts) removeNode(n->mID);
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
	foreach (ScaffNode* n, splitNodes) chainParts << (PatchNode*)n; 

	// sort them
	QMap<double, PatchNode*> distPartMap;
	Geom::Plane base_plane = baseMaster->mPatch.getPlane();
	foreach(PatchNode* n, chainParts)
	{
		double dist = base_plane.signedDistanceTo(n->center());
		distPartMap[fabs(dist)] = n;
	}
	chainParts = distPartMap.values().toVector();

	// debug
	//addDebugSegment(getShrunkTopTraj());
	//addDebugSegment(getShrunkSlaveSeg());
	properties[DEBUG_PLANES].setValue(cutPlanes);
}

void ChainScaff::resetHingeLinks(FoldOption* fn)
{
	// remove hinge links
	rightLinks.clear(); leftLinks.clear();
	foreach (Structure::Link* link, links)
		Structure::Graph::removeLink(link);

	// hinge links between base and slave
	Geom::Segment bJoint = baseJoint;
	Vector3 upV = slaveSeg.Direction;
	Vector3 jointV = bJoint.Direction;
	if(chainParts.empty()) return;
	Hinge* hingeR = new Hinge(chainParts[0], baseMaster, bJoint.P0, upV,  rightSegV, jointV, bJoint.length());


	//std::cout << "\nCreating Hinges \nupV = ";
	//print(upV);
	//std::cout << "rightV = ";
	//print(rightSegV);
	//std::cout << "jointV = ";
	//print(jointV);


	Hinge* hingeL = new Hinge(chainParts[0], baseMaster, 
		bJoint.P1, upV, -rightSegV, -jointV, bJoint.length());

	ScaffLink* linkR = new ScaffLink(chainParts[0], baseMaster, hingeR);
	ScaffLink* linkL = new ScaffLink(chainParts[0], baseMaster, hingeL);
	Graph::addLink(linkR);	
	Graph::addLink(linkL);
	rightLinks << linkR; 
	leftLinks << linkL;

	// hinge links between two parts in the chain
	double l = slaveSeg.length();
	double d = rightSeg.length();
	double step = (l - d) / (fn->nSplits + 1);
	if (!fn->rightSide){
		bJoint.translate(d * slaveSeg.Direction);
	}

	for (int i = 1; i < chainParts.size(); i++)
	{
		PatchNode* part1 = chainParts[i];
		PatchNode* part2 = chainParts[i-1];

		bJoint.translate(step * slaveSeg.Direction);
		Vector3 upV = slaveSeg.Direction;
		Vector3 jointV = bJoint.Direction;
		Hinge* hingeR = new Hinge(part1, part2, 
			bJoint.P0, upV, -upV, jointV, bJoint.length());
		Hinge* hingeL = new Hinge(part1, part2, 
			bJoint.P1, upV, -upV, -jointV, bJoint.length());

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


void ChainScaff::addThickness(Scaffold* keyframe, double t)
{
	if (!baseMaster) return;

	// get parts in key frame
	QVector<PatchNode*> keyParts;
	keyParts << (PatchNode*)keyframe->getNode(baseMaster->mID);
	foreach(PatchNode* cpart, chainParts) keyParts << (PatchNode*)keyframe->getNode(cpart->mID);
	keyParts << (PatchNode*)keyframe->getNode(topMaster->mID);

	// move parts up for thickness
	for (int i = 1; i < keyParts.size(); i++)
	{
		// compute offset caused by thickness between part_i and part_i_1
		Vector3 offset(0, 0, 0);
		Vector3 v1(0, 0, 0), v2(0, 0, 0);
		if (i == 1)
		{
			// base
			Geom::Rectangle rect1 = keyParts[i]->mPatch;
			v2 = keyParts[0]->mPatch.Normal; //up

			double dotNN = dot(rect1.Normal, v2);
			if (dotNN > ZERO_TOLERANCE_LOW) v1 = rect1.Normal;
			else if (dotNN < -ZERO_TOLERANCE_LOW) v1 = -rect1.Normal;

			offset = halfThk * v1 + baseOffset * v2;
		}
		else if (i == keyParts.size() - 1)
		{
			// top
			v1 = keyParts[0]->mPatch.Normal; //up
			Geom::Rectangle rect2 = keyParts[i - 1]->mPatch;

			double dotNN = dot(v1, rect2.Normal);
			if (dotNN > ZERO_TOLERANCE_LOW) v2 = -rect2.Normal;
			else if (dotNN < -ZERO_TOLERANCE_LOW) v2 = rect2.Normal;

			offset = halfThk * (v1 - v2);
		}
		else
		{
			// chain parts
			Geom::Rectangle rect1 = keyParts[i]->mPatch;
			Geom::Rectangle rect2 = keyParts[i - 1]->mPatch;

			// flattened
			if (1 - t < ZERO_TOLERANCE_LOW)
			{
				v1 = baseMaster->mPatch.Normal;
				v2 = -v1;
			}
			else
			{
				int side2to1 = rect1.whichSide(rect2.Center);
				if (side2to1 == 1) v1 = -rect1.Normal;
				else if (side2to1 == -1) v1 = rect1.Normal;

				int side1to2 = rect2.whichSide(rect1.Center);
				if (side1to2 == 1) v2 = -rect2.Normal;
				else if (side1to2 == -1) v2 = rect2.Normal;
			}

			offset = halfThk * (v1 - v2);
		}

		// translate parts above
		for (int j = i; j < keyParts.size(); j++)
			keyParts[j]->translate(offset);
	}

	// set thickness to chain parts
	for (int i = 1; i < keyParts.size() - 1; i++)
		keyParts[i]->setThickness(2 * halfThk);
}

double ChainScaff::getArea()
{
	return origSlave->mPatch.area();
}
