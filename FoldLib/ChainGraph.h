#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "DependGraph.h"

class ChainGraph : public FdGraph
{
public:
    ChainGraph(FdNode* part, PatchNode* panel1, PatchNode* panel2 = NULL);
	
	virtual void fold(FoldingNode* fn) = 0;

public:
	FdNode		*mPart;
	PatchNode	*mPanel1, *mPanel2;

	QVector<Geom::Segment> hingeSegs;
	Geom::Segment upSeg; // perp segment on part
	QVector<Vector3> rightVs; // perp direction on panel to the right
	int nbRods;
};