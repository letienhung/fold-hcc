#pragma once

#include "Scaffold.h"
#include "PatchNode.h"
#include "FoldOptGraph.h"

class ChainScaff : public Scaffold
{
public:
    ChainScaff(ScaffNode* slave, PatchNode* base, PatchNode* top);
	~ChainScaff();

	// set up
	void computeOrientations();

	// time interval
	void setFoldDuration(double t0, double t1);

	// thickness
	void addThickness(Scaffold* keyframe, double t);

	// the area of original slave patch
	double getSlaveArea();

public:
	// fold options
	FoldOption* genFoldOption(QString id, bool isRight, double width, double startPos, int nSplits); // regular option
	FoldOption* genDeleteFoldOption();	// deleting option
	QVector<FoldOption*> genRegularFoldOptions(int nS, int nC);	// regular options with nS and nC
	virtual QVector<FoldOption*> genRegularFoldOptions() = 0;	// all regular options

	// fold region
	virtual Geom::Rectangle getFoldRegion(FoldOption* fn) = 0;

	// apply fold option: modify the chain
	void applyFoldOption(FoldOption* fn);
	void resetChainParts(FoldOption* fn);
	void resetHingeLinks(FoldOption* fn);
	void activateLinks(FoldOption* fn);
	virtual QVector<Geom::Plane> generateCutPlanes(FoldOption* fn) = 0;

	// folding
	virtual void fold(double t) = 0;

	// key frame
	Scaffold* getKeyframe(double t, bool useThk);

public:
	PatchNode*			topMaster;	// top
	PatchNode*			baseMaster;	// base							
	PatchNode*			origSlave;	// original slave
	QVector<PatchNode*>	chainParts;	// sorted parts in the chain, from base to top

	/*
		topJoint				topCenter
			|\						^
			: \						|
			:  \ slaveSeg			| upSeg
			:   \					|
			:    \					|
			:-----> (x)baseJoint	|
	   rightSeg/rightDirect		baseRect
	*/
	Geom::Segment		baseJoint;	// joint between slave and base
	Geom::Segment		topJoint;	// joint between slave and top
	Geom::Segment		slaveSeg;	// 2D abstraction, perp to joints (base to top)
	Geom::Segment		rightSeg;	// right direction, perp to joints
	Geom::Segment		upSeg;		// up right segment from topCenter's projection to topCenter
	Vector3				rightDirect;// direction of rightSeg; or the cross product of baseJoint and slaveSeg

	QVector<ScaffLink*>	rightLinks;	// right hinges 
	QVector<ScaffLink*>	leftLinks;	// left hinges
	QVector<ScaffLink*>	activeLinks;// active hinges

	TimeInterval		duration;	// time interval
	bool				foldToRight;// folding side

	bool				isDeleted;	// deleted fold option has been applied to this chain
	double				importance;	// normalized importance wrt. patch area


	// thickness
	double topHThk, baseHThk, slaveHThk;	// *** half thickness
};