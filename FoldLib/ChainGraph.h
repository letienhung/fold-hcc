#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "FoldOptionGraph.h"

class ChainGraph : public FdGraph
{
public:
	enum CHAIN_TYPE{T_CHAIN, H_CHAIN};

public:
	// constructor
    ChainGraph(FdNode* slave, PatchNode* master1, PatchNode* master2);
	void setupBasisOrientations();

	// foldem
	QVector<FoldOption*> generateFoldOptions(int nSplits, int nUsedChunks, int nChunks);

	// modify chain
	virtual void applyFoldOption(FoldOption* fn);
	void createSlavePart(FoldOption* fn);
	virtual QVector<Geom::Plane> generateCutPlanes(int nbSplit) = 0;
	void sortChainParts();
	void resetHingeLinks();
	void setupActiveLinks(FoldOption* fn);

	// animation
	void fold(double t);
	FdGraph* getKeyframe(double t);

	// setter
	void setFoldDuration(double t0, double t1);

public:
	CHAIN_TYPE mType;

	QVector<PatchNode*>		mMasters;		// one or two masters: from low to high
	Geom::Segment			mMC2Trajectory;	// segment from m2's center to its projection on m1
											// the trajectory of m2's center during folding

	FdNode*					mOrigSlave;		// original part, which is split into chain parts
	QVector<FdNode*>		mParts;			// sorted parts in the chain, from masters[0] to masters[1]

	QVector<Geom::Segment>	rootJointSegs;	// hinge segments between mOrigPart and masters[0]
	Geom::Segment			chainUpSeg;		// perp segment on mOrigPart
	QVector<Vector3>		rootRightVs;	// perp direction on masters[0] to the right

	// each joint has one(patch) or two(rod) joint axis
	// for each joint axis, there are two hinge links
	// \hingeLinks: 
	//		1st dimension: joint
	//		2nd dimension: hinge pairs [right: 2*j, left: 2*j+1] for joint axis j
	QVector< QVector<FdLink*> > hingeLinks;
	QVector<FdLink*> activeLinks;
	TimeInterval mFoldDuration;

	// thickness
	double half_thk;
	double base_offset;
};