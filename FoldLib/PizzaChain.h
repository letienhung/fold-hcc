#pragma once

#include "Segment.h"
#include "FdGraph.h"
#include "PatchNode.h"

class PizzaChain : public FdGraph
{
public:
    PizzaChain(FdNode* part, PatchNode* panel, QString id);

public:
	FdNode* mPart;
	PatchNode* mPanel;

	QVector<Geom::Segment>  hinges; 
};
