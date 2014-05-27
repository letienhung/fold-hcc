#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "FoldOptionGraph.h"

class ChainGraph : public FdGraph
{
public:
	// constructor
    ChainGraph(FdNode* slave, PatchNode* base, PatchNode* top);

	// fold options
	QVector<FoldOption*> generateFoldOptions(int nSplits, int nUsedChunks, int nChunks);

	// fold region
	Geom::Rectangle getFoldRegion(FoldOption* fn);
	Geom::Rectangle getMaxFoldRegion(bool right);

	// modify chain
	void applyFoldOption(FoldOption* fn);
	void createSlavePart(FoldOption* fn);
	QVector<Geom::Plane> generateCutPlanes(int nbSplit);
	void sortChainParts();
	void resetHingeLinks();
	void setActiveLinks(FoldOption* fn);

	// animation
	void fold(double t);
	FdGraph* getKeyframe(double t);

	// setter
	void setFoldDuration(double t0, double t1);

public:
	PatchNode*			topMaster;
	PatchNode*			baseMaster;
	Geom::Segment		mMC2Trajectory;	// segment from m2's center to its projection on m1
										// the trajectory of m2's center during folding

	PatchNode*			mOrigSlave;		// original slave, which is split into chain parts
	QVector<PatchNode*>	mParts;			// sorted parts in the chain, from base to top

	Geom::Segment		baseJoint;		// joint segment between slave and base
	Geom::Segment		topJoint;		// joint segment between slave and top
	Geom::Segment		UpSeg;		// perp segment on slave
	Vector3				rightV;			// perp direction on base to the right

	// for each joint, there are two hinges: left and right
	QVector<FdLink*> rightLinks;
	QVector<FdLink*> leftLinks;
	QVector<FdLink*> activeLinks;
	TimeInterval mFoldDuration;

	// thickness
	double half_thk;
	double base_offset;
};