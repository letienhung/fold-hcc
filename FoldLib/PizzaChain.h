#pragma once

#include "Segment.h"
#include "FdGraph.h"

class PizzaChain : public FdGraph
{
public:
    PizzaChain(FdNode* part, FdNode* panel, QString id);

public:
	FdNode* mPart;
	FdNode* mPanel;
};
