#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "FoldOptionGraph.h"

class ChainGraph : public FdGraph
{
public:
	// constructor
    ChainGraph(FdNode* part, PatchNode* panel1, PatchNode* panel2);
	void setupBasisOrientations();

	// fold options
	virtual QVector<FoldingNode*> generateFoldOptions() = 0;
	virtual void modify(FoldingNode* fn) = 0;

	// Modify chain
	void createChain(int N);
	void sortChainParts();
	void resetHingeLinks();
	void shrinkChainAlongJoint(double t0, double t1);
	void shrinkChainPerpJoint();

	void setupActiveLinks(FoldingNode* fn);
	void fold(double t);

	// animation
	QVector<Structure::Node*> getKeyframeParts(double t);
	QVector<Structure::Node*> getKeyFramePanels(double t);

	double getLength();
	QVector<Geom::Plane> generateCutPlanes(int N);

public:
	QVector<PatchNode*>		mMasters;		// one or two masters
	FdNode*					mOrigPart;	// original part, which is split into chain parts
	QVector<FdNode*>		mParts;			// sorted parts in the chain, from panels[0] to panels[1]

	QVector<Geom::Segment>	rootJointSegs;	// hinge segments between mOrigPart and panels[0]
	Geom::Segment			chainUpSeg;		// perp segment on mOrigPart
	QVector<Vector3>		rootRightVs;	// perp direction on panels[0] to the right

	// each joint corresponds one or two joint segments
	// for each joint segment, there are two hinge links
	// \hingeLinks: 
	//		1st dimension: joint
	//		2nd dimension: hinge pairs [2*i, 2*i+1] for each jointSeg[i]
	QVector< QVector<FdLink*> > hingeLinks;
	QVector<FdLink*> activeLinks;
};