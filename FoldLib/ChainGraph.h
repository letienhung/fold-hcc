#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "DependGraph.h"

class ChainGraph : public FdGraph
{
public:
    ChainGraph(FdNode* part, PatchNode* panel1, PatchNode* panel2 = NULL);
	
	virtual void prepareFolding(FoldingNode* fn) = 0;
	virtual void fold(double t) = 0;

	QVector<Structure::Node*> getKeyframeParts(double t);
	QVector<Structure::Node*> getKeyFramePanels(double t);

public:
	QVector<PatchNode*>		mPanels;		// two control panels
	QVector<FdNode*>		mParts;			// sorted parts in the chain, from panels[0] to panels[1]

	QVector<Geom::Segment>	rootJointSegs;	// hinge segments between parts[0] and panels[0]
	Geom::Segment			chainUpSeg;		// perp segment on part
	QVector<Vector3>		rootRightVs;	// perp direction on panel to the right

	// each joint corresponds one or two joint segments
	// for each joint segment, there are two hinge links
	// \hingeLinks: 
	//		1st dimension: joint
	//		2nd dimension: hinge pairs [2*i, 2*i+1] for each jointSeg[i]
	int nbRods;
	QVector< QVector<FdLink*> > hingeLinks;

	bool isReady;
	QVector<FdLink*> activeLinks;
};