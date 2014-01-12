#pragma once

#include "ChainGraph.h"
#include "Rectangle2.h"

class SandwichChain : public ChainGraph
{
public:
    SandwichChain(FdNode* part, PatchNode* panel1, PatchNode* panel2);

	Geom::Rectangle2 getFoldingArea(FoldingNode* fn);
	Geom::Segment2 getFoldingAxis2D(FoldingNode* fn);

	Geom::Segment getJointSegment(FoldingNode* fn);

	void resolveCollision(FoldingNode* fn);
};

