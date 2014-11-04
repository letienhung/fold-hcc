#include "TChainScaff.h"
#include "Numeric.h"

TChainScaff::TChainScaff(ScaffNode* slave, PatchNode* base, PatchNode* top)
:ChainScaff(slave, base, top)
{
}

Geom::Rectangle TChainScaff::getFoldRegion(FoldOption* fn)
{
	Geom::Rectangle base_rect = baseMaster->mPatch;

	double l = slaveSeg.length();
	double offset = l / (fn->nSplits + 1);

	Geom::Segment rightSeg, leftSeg;
	if (fn->rightSide)
	{
		leftSeg = baseJoint;
		rightSeg = baseJoint.translated(offset * rightDirect);
	}
	else
	{
		leftSeg = baseJoint.translated(-offset * rightDirect);
		rightSeg = baseJoint;
	}

	// shrink along jointV
	double t0 = fn->position;
	double t1 = t0 + fn->scale;
	leftSeg.cropRange01(t0, t1);
	rightSeg.cropRange01(t0, t1);

	// fold region in 3D
	Geom::Rectangle region(QVector<Vector3>()
		<< leftSeg.P0 << leftSeg.P1
		<< rightSeg.P1 << rightSeg.P0);

	return region;
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
QVector<Geom::Plane> TChainScaff::generateCutPlanes(FoldOption* fn)
{
	// start plane and step
	double h = upSeg.length();
	double step = h / (fn->nSplits + 1);
	Geom::Plane start_plane = baseMaster->mPatch.getPlane();

	// the normal of base master points to the same side as the chain (see TBlockGraph)
	Vector3 stepV = step * start_plane.Normal;

	// cut planes
	QVector<Geom::Plane> cutPlanes;
	for (int i = 1; i <= fn->nSplits; i++)
	{
		Vector3 deltaV = i * stepV;
		cutPlanes << start_plane.translated(deltaV);
	}

	return cutPlanes;
}

void TChainScaff::fold(double t)
{
	// free all nodes
	for (Structure::Node* n : nodes)
		n->removeTag(FIXED_NODE_TAG);

	// fix base
	baseMaster->addTag(FIXED_NODE_TAG);

	// set angle for active link
	activeLinks[0]->hinge->angle = getRootAngle() * (1 - t);
	for (int i = 1; i < activeLinks.size(); i++)
		activeLinks[i]->hinge->angle = M_PI * (1 - t);

	// restore configuration
	restoreConfiguration();
}

QVector<FoldOption*> TChainScaff::genRegularFoldOptions(int maxNbSplits, int maxNbChunks)
{
	// enumerate all possible combination of nS and nC
	QVector<FoldOption*> options;
	for (int nS = 0; nS <= maxNbSplits; nS ++)
	for (int nC = 1; nC <= maxNbChunks; nC++)
		options << ChainScaff::genRegularFoldOptions(nS, nC, maxNbChunks);

	return options;
}

double TChainScaff::getRootAngle()
{
	Vector3 initV = slaveSeg.Direction;
	Vector3 finalV = foldToRight ? rightDirect : -rightDirect;
	return acos(RANGED(-1, dot(initV, finalV), 1));
}
