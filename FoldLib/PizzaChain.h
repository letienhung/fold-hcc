#pragma once

#include "ChainGraph.h"
#include "SectorCylinder.h"

class PizzaChain : public ChainGraph
{
public:
    PizzaChain(FdNode* part, PatchNode* panel);

	QVector<FoldingNode*> generateFoldOptions();
	void modify(FoldingNode* fn);

	Geom::SectorCylinder getFoldingVolume(FoldingNode* fn);

	void resolveCollision(FoldingNode* fn);
};
