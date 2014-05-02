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
	CHAIN_TYPE mType;

	QVector<PatchNode*>		mMasters;		// one or two masters
	FdNode*					mOrigSlave;		// original part, which is split into chain parts
	QVector<FdNode*>		mParts;			// sorted parts in the chain, from masters[0] to masters[1]

	QVector<Geom::Segment>	rootJointSegs;	// hinge segments between mOrigPart and masters[0]
	Geom::Segment			chainUpSeg;		// perp segment on mOrigPart
	QVector<Vector3>		rootRightVs;	// perp direction on masters[0] to the right

	// each joint corresponds to one(patch) or two(rod) joint segments
	// for each joint segment, there are two hinge links
	// \hingeLinks: 
	//		1st dimension: joint
	//		2nd dimension: hinge pairs [2*i, 2*i+1] for each jointSeg[i]
	QVector< QVector<FdLink*> > hingeLinks;
	QVector<FdLink*> activeLinks;
};