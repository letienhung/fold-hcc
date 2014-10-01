#pragma once

#include "Scaffold.h"
#include "PatchNode.h"
#include "FoldOptionGraph.h"

class ChainScaffold : public Scaffold
{
public:
    ChainScaffold(ScaffoldNode* slave, PatchNode* base, PatchNode* top);
	~ChainScaffold();

	// set up
	void computeOrientations();

	// time interval
	void setFoldDuration(double t0, double t1);

	// thickness
	void addThickness(Scaffold* keyframe, double t);

	// the area of original slave patch
	double getArea();

public:
	// fold options
	FoldOption* genDeleteFoldOption(int nSplits);
	QVector<FoldOption*> genFoldOptionWithDiffPositions(int nSplits, int nChunks, int maxNbChunks);
	virtual QVector<FoldOption*> genRegularFoldOptions(int nSplits, int nChunks) = 0;

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
			:  \ slaveSeg			| topTraj
			:   \					|
			:    \					|
			:-----> (x)baseJoint	|
		    rightSeg			baseRect
	*/
	Geom::Segment		baseJoint;	// joint between slave and base
	Geom::Segment		topJoint;	// joint between slave and top
	Geom::Segment		slaveSeg;	// 2D abstraction, perp to joints (base to top)
	Geom::Segment		rightSeg;	// right direction, perp to joints
	Vector3				rightSegV;	// direction of rightSeg
	Geom::Segment		topTraj;	// trajectory of top's center during folding (base to top)

	QVector<ScaffoldLink*>	rightLinks;	// right hinges 
	QVector<ScaffoldLink*>	leftLinks;	// left hinges
	QVector<ScaffoldLink*>	activeLinks;// active hinges

	TimeInterval		duration;	// time interval
	bool				foldToRight;// folding side


	//			topJoint				
	//	half_thk  __|\___________			
	//				::\				
	//				:: \ 		
	//				::  \			
	//				::   \			
	//				::	  \	slaveSeg		
	//				::     \			
	//				::      \			
	//	     _______::_______\_______			
	//	baseOffset	:--------->
	//				 rightSeg	
	double halfThk;		// thickness of slave and top master
	double baseOffset;	// offset caused by thickness of base master and its super siblings

	bool isDeleted;		// deleted fold option has been applied to this chain
};