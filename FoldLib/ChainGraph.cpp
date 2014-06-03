#include "ChainGraph.h"
#include "FdUtility.h"
#include "RodNode.h"
#include "Numeric.h"

ChainGraph::ChainGraph( FdNode* slave, PatchNode* base, PatchNode* top)
	: FdGraph(slave->mID), baseMaster(NULL), origSlave(NULL), topMaster(NULL)
{
	// slave
	if (slave->mType == FdNode::ROD)
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

	// set up orientations
	computeOrientations();

	// fold duration
	duration = INTERVAL(1, 2);

	// thickness
	halfThk = 0;
	baseOffset = 0;

	// side
	foldToRight = true;

	// patch area
	patchArea = origSlave->mPatch.area();

	//// debug
	//addDebugSegment(baseJoint);
	//addDebugSegment(slaveSeg);
	//addDebugSegment(rightSeg);
	//addDebugSegment(Geom::Segment(baseJoint.P0, baseJoint.P0 + rightSegV));
}

ChainGraph::~ChainGraph()
{

}

void ChainGraph::computeOrientations()
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

void ChainGraph::computePhaseSeparator()
{
	// constant
	int n = chainParts.size();
	double L = slaveSeg.length();
	double d = rightSeg.length();
	double a = (L - d) / n;
	double b = d + a;
	double A = (n - 1) * a;

	// height
	heightSep = A + sqrt(b * b - d * d);

	// angle
	double sin_angle = heightSep - A;
	angleSep = asin(RANGED(0, sin_angle, 1));
}

void ChainGraph::foldUniformAngle( double t )
{
	// hinge angles
	double d = rightSeg.length();
	double sl = slaveSeg.length();
	int n = chainParts.size();
	double alpha = acos(d / sl) * (1 - t);

	double a = (sl - d) / n;
	double b = d + a;
	double bProj = b * cos(alpha);
	double beta;
	// no return
	if (bProj <= d)
	{
		double cos_beta = (d - bProj)/((n - 1) * a);
		beta = acos(cos_beta);

		if (foldToRight)
		{
			activeLinks[0]->hinge->angle = M_PI - beta;
			for (int i = 1; i < activeLinks.size()-1; i++)
				activeLinks[i]->hinge->angle = M_PI;
			activeLinks.last()->hinge->angle = alpha + M_PI - beta;
		}
		else
		{
			activeLinks[0]->hinge->angle = alpha;
			activeLinks[1]->hinge->angle = alpha + M_PI - beta;
			for (int i = 2; i < activeLinks.size(); i++)
				activeLinks[i]->hinge->angle = M_PI;
		}
	}
	// return
	else
	{
		double cos_beta = (b * cos(alpha) - d) / a;
		beta = acos(RANGED(0, cos_beta, 1));

		if(!activeLinks.size()) return;

		if (foldToRight)
		{
			activeLinks[0]->hinge->angle = beta;
			for (int i = 1; i < activeLinks.size()-1; i++)
				activeLinks[i]->hinge->angle = 2 * beta;
			activeLinks.last()->hinge->angle = alpha + beta;
		}
		else
		{
			activeLinks[0]->hinge->angle = alpha;
			activeLinks[1]->hinge->angle = alpha + beta;
			for (int i = 2; i < activeLinks.size(); i++)
				activeLinks[i]->hinge->angle = 2 * beta;
		}
	}

	// adjust the position of top master
	double ha = a * sin(beta);
	double hb = b * sin(alpha);
	double topH = (n - 1) * ha + hb;
	Vector3 topPos = topTraj.P0 + topH * topTraj.Direction;
	topMaster->translate(topPos - topMaster->center());
}

void ChainGraph::foldUniformHeight( double t )
{
	// constant
	int n = chainParts.size();
	double L = slaveSeg.length();
	double d = rightSeg.length();
	double a = (L - d) / n;
	double b = d + a;
	double A = (n - 1) * a;
	double H = topTraj.length();
	double h = (1 - t) * H;
	// phase-I: no return
	if (h >= heightSep)
	{
		double alpha_orig = acos(RANGED(0, d/L ,1));
		Interval alpha_it = INTERVAL(angleSep, alpha_orig);
		double x = 2 * b * h;
		double y = 2 * b * d;
		double z = d * d + h * h + b * b - A * A;
		double aa = x * x + y * y;
		double bb = -2 * y * z;
		double cc = z * z - x * x;

		double alpha = 0;
		QVector<double> roots = findRoots(aa, bb, cc);
		foreach (double r, roots){
			alpha = acos(RANGED(0, r, 1));
			if (within(alpha, alpha_it)) break;
		}
		if (alpha == 0) alpha = alpha_orig; //set original angle if no roots found

		double cos_beta = (d - b * cos(alpha)) / A;
		double beta = acos(RANGED(0, cos_beta, 1));

		// set angles
		if (foldToRight)
		{
			activeLinks[0]->hinge->angle = M_PI - beta;
			for (int i = 1; i < activeLinks.size()-1; i++)
				activeLinks[i]->hinge->angle = M_PI;
			activeLinks.last()->hinge->angle = alpha + M_PI - beta;
		}
		else
		{
			activeLinks[0]->hinge->angle = alpha;
			activeLinks[1]->hinge->angle = alpha + M_PI - beta;
			for (int i = 2; i < activeLinks.size(); i++)
				activeLinks[i]->hinge->angle = M_PI;
		}
	}
	// phase-II: return
	else
	{
		Interval alpha_it = INTERVAL(0, angleSep);
		double B =  b * b * n * (n-2);
		double C = -2 * b * d * (n-1) * (n-1);
		double D = A * A - h * h - b * b - (n-1) * (n-1) * d * d;
		double E = 4 * b * b * h * h - 2 * B * D + C * C;
		double F = -2 * C * D;
		double G = D * D - 4 * b * b * h * h;
		double K = 2 * B * C;

		double alpha = 0, beta = 0;
		QVector<double> roots = findRoots(B * B, K, E, F, G);
		foreach (double r, roots)
		{
			if (r < 0) continue;
	
			alpha = acos(RANGED(0, r, 1));
			if (within(alpha, alpha_it))
			{
				double cos_beta = (b * r - d) / a;
				beta = acos(RANGED(0, cos_beta, 1));

				double hh = A * sin(beta) + b * sin(alpha);
				if (fabs(h - hh) < ZERO_TOLERANCE_LOW)
					break;
			}
		}
		// angles are sensitive to noise at very beginning or end
		if (t < 0.01)
		{	
			alpha = angleSep; 
			double cos_beta = (b * cos(alpha) - d) / a;
			beta = acos(RANGED(0, cos_beta, 1));
		}
		if (1 - t < 0.01)
		{
			alpha = 0;
			double cos_beta = (b * cos(alpha) - d) / a;
			beta = acos(RANGED(0, cos_beta, 1));
		}

		// set angles
		if (foldToRight)
		{
			activeLinks[0]->hinge->angle = beta;
			for (int i = 1; i < activeLinks.size()-1; i++)
				activeLinks[i]->hinge->angle = 2 * beta;
			activeLinks.last()->hinge->angle = alpha + beta;
		}
		else
		{
			activeLinks[0]->hinge->angle = alpha;
			activeLinks[1]->hinge->angle = alpha + beta;
			for (int i = 2; i < activeLinks.size(); i++)
				activeLinks[i]->hinge->angle = 2 * beta;
		}
	}

	// adjust the position of top master
	Vector3 topPos = topTraj.P0 + h * topTraj.Direction;
	topMaster->translate(topPos - topMaster->center());
}

void ChainGraph::addThickness(FdGraph* keyframe, double t)
{
	if(!baseMaster) return;

	// get parts in key frame
	QVector<PatchNode*> keyParts;
	keyParts << (PatchNode*)keyframe->getNode(baseMaster->mID);
	foreach (PatchNode* cpart, chainParts) keyParts << (PatchNode*)keyframe->getNode(cpart->mID);
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
			if ( dotNN > ZERO_TOLERANCE_LOW) v1 = rect1.Normal;
			else if (dotNN < -ZERO_TOLERANCE_LOW) v1 = -rect1.Normal;

			offset = halfThk * v1 + baseOffset * v2;
		}
		else if (i == keyParts.size() - 1)
		{
			// top
			v1 = keyParts[0]->mPatch.Normal; //up
			Geom::Rectangle rect2 = keyParts[i-1]->mPatch;
			
			double dotNN = dot(v1, rect2.Normal);
			if (dotNN > ZERO_TOLERANCE_LOW) v2 = -rect2.Normal;
			else if (dotNN < -ZERO_TOLERANCE_LOW) v2 = rect2.Normal;

			offset = halfThk * (v1 - v2);
		}
		else
		{
			// chain parts
			Geom::Rectangle rect1 = keyParts[i]->mPatch;
			Geom::Rectangle rect2 = keyParts[i-1]->mPatch;

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
	for (int i = 1; i < keyParts.size()-1; i++)
		keyParts[i]->setThickness(2 * halfThk);
}

void ChainGraph::fold( double t )
{
	if(!baseMaster) return;

	// free all nodes
	foreach (Structure::Node* n, nodes)
		n->removeTag(FIXED_NODE_TAG);

	// fix base
	baseMaster->addTag(FIXED_NODE_TAG);

	// set up hinge angles and top position
	//foldUniformHeight(t);
	foldUniformAngle(t);

	// restore configuration
	restoreConfiguration();
}

FdGraph* ChainGraph::getKeyframe( double t, bool useThk )
{
	// fold w\o thickness
	fold(t);

	// key frame
	FdGraph* keyframe = (FdGraph*)this->clone();

	// thickness
	if (useThk) addThickness(keyframe, t);

	return keyframe;
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
		FoldOption* fn1 = new FoldOption(fnid1, false, usedSize, position, nSplits, patchArea);
		options.push_back(fn1);

		// right
		QString fnid2 = QString("%1:%2_%3_R_%4").arg(mID).arg(nSplits).arg(nUsedChunks).arg(position);
		FoldOption* fn2 = new FoldOption(fnid2, true, usedSize, position, nSplits, patchArea);
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

	// debug
	//addDebugSegment(getShrunkTopTraj());
	//addDebugSegment(getShrunkSlaveSeg());
}

void ChainGraph::resetHingeLinks(FoldOption* fn)
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

	FdLink* linkR = new FdLink(chainParts[0], baseMaster, hingeR);
	FdLink* linkL = new FdLink(chainParts[0], baseMaster, hingeL);
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

	computePhaseSeparator();

	// statistics
	nbHinges = fn->nSplits + 2;
	shrinkedArea = fn->patchArea * (1 - fn->scale);
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
	if(!baseMaster) return Geom::Rectangle();

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

	// debug
	addDebugSegments(region.getEdgeSegments());

	return region;
}

Geom::Rectangle ChainGraph::getMaxFoldRegion( bool right )
{
	FoldOption fn("", right, 1.0, 0.0, 1, patchArea);
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
	// constants
	double L = slaveSeg.length();
	double d = rightSeg.length();
	double h = topTraj.length();
	double a = (L - d) / (fn->nSplits + 1);
	double sin_alpha = h / L;
	double step = a * sin_alpha;

	// cut at lower end
	Geom::Plane plane0 =  baseMaster->mPatch.getPlane();
	Vector3 stepV = step * plane0.Normal;
	// cut at higher end
	if (!fn->rightSide) {
		plane0.translate(topTraj.P1 - topTraj.P0);
		stepV *= -1;
	}

	// cut planes
	QVector<Geom::Plane> cutPlanes;
	for (int i = 1; i <= fn->nSplits; i++)
	{
		Vector3 deltaV = i * stepV;
		cutPlanes << plane0.translated(deltaV);
	}

	return cutPlanes;
}