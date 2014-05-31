#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "FoldOptionGraph.h"

class ChainGraph : public FdGraph
{
public:
    ChainGraph(FdNode* slave, PatchNode* base, PatchNode* top);

	// set up
	void computeOrientations();
	void computePhaseSeparator();

	// fold options
	QVector<FoldOption*> generateFoldOptions(int nSplits, int nUsedChunks, int nChunks);
	Geom::Rectangle getFoldRegion(FoldOption* fn);
	Geom::Rectangle getMaxFoldRegion(bool right);

	// modify chain
	void applyFoldOption(FoldOption* fn);
	void resetChainParts(FoldOption* fn);
	void resetHingeLinks(FoldOption* fn);
	void setActiveLinks(FoldOption* fn);

	// animation
	void fold(double t);
	void foldUniformHeight(double t);
	void foldUniformAngle(double t);

	// key frame
	FdGraph* getKeyframe(double t);
	void setFoldDuration(double t0, double t1);

	// helpers
	Interval getShrunkInterval();
	double getShunkScale();
	Geom::Segment getShrunkSlaveSeg();
	Geom::Segment getShrunkTopTraj();
	QVector<Geom::Plane> generateCutPlanes(FoldOption* fn);

public:
	PatchNode*			topMaster;	// top
	PatchNode*			baseMaster;	// base							
	PatchNode*			origSlave;	// original slave
	QVector<PatchNode*>	chainParts;	// sorted parts in the chain, from base to top

	//	topJoint				topCenter
	//		|\						^
	//		: \						|
	//		:  \ slaveSeg			| topTraj
	//		:   \					|
	//		:    \					|
	//		:-----> (x)baseJoint	|
	//	    rightSeg			baseRect
	Geom::Segment		baseJoint;	// joint between slave and base
	Geom::Segment		topJoint;	// joint between slave and top
	Geom::Segment		slaveSeg;	// 2D abstraction, perp to joints (base to top)
	Geom::Segment		rightSeg;	// right direction, perp to joints
	Vector3				rightSegV;	// direction of rightSeg
	Geom::Segment		topTraj;	// trajectory of top's center during folding (base to top)

	QVector<FdLink*>	rightLinks;	// right hinges 
	QVector<FdLink*>	leftLinks;	// left hinges
	QVector<FdLink*>	activeLinks;// active hinges
	Interval			duration;	// time interval
	bool				foldToRight;// folding side
	bool				isInclined; // inclined
	double				heightSep;	// the height separates phase I and II
	double				angleSep;	// angle between b and base

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
};