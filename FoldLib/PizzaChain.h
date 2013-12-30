#pragma once

#include "ChainGraph.h"
#include "SectorCylinder.h"

class PizzaChain : public ChainGraph
{
public:
    PizzaChain(FdNode* part, PatchNode* panel);

	Geom::SectorCylinder getFoldingVolume(FoldingNode* fn);

	void fold(FoldingNode* fn);
};
