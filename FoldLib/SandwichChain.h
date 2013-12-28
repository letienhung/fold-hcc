#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "Rectangle.h"

class SandwichChain : public FdGraph
{
public:
    SandwichChain(FdNode* part, PatchNode* panel1, PatchNode* panel2, QString id);

public:
	FdNode *mPart;
	PatchNode *mPanel1, *mPanel2;

	QVector<Geom::Segment> hinges;


};

