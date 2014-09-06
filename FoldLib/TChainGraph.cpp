#include "TChainGraph.h"

TChainGraph::TChainGraph(FdNode* slave, PatchNode* base, PatchNode* top)
:ChainGraph(slave, base, top)
{
	// do nothing
}

Geom::Rectangle TChainGraph::getFoldRegion(FoldOption* fn)
{
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Segment topJointProj = base_rect.getProjection(topJoint);

	double l = slaveSeg.length();
	double offset = l / (fn->nSplits + 1);

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
		<< leftSeg.P0 << leftSeg.P1
		<< rightSeg.P1 << rightSeg.P0);

	// debug
	//addDebugSegments(region.getEdgeSegments());

	return region;
}

Geom::Rectangle TChainGraph::getMaxFoldRegion(bool isRight)
{
	FoldOption fn("", isRight, 1.0, 0.0, 0, patchArea);
	return getFoldRegion(&fn);
}
