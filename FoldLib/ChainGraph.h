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
	FdLink		*mLink1, *mLink2;

	double mLength;
	int nbRods;
};