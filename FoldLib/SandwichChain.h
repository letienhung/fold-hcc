#pragma once

#include "FdGraph.h"

class SandwichChain : public FdGraph
{
public:
    SandwichChain(FdNode* part, FdNode* panel1, FdNode* panel2, QString id);

public:
	FdNode *mPart;
	FdNode *mPanel1, *mPanel2;
};

