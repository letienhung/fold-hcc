#pragma once

#include "ChainGraph.h"
#include "SectorCylinder.h"

class TChain : public ChainGraph
{
public:
    TChain(PatchNode* master, FdNode* slave);

	QVector<FoldingNode*> generateFoldOptions();
	void modify(FoldingNode* fn);

	Geom::SectorCylinder getFoldingVolume(FoldingNode* fn);

	void resolveCollision(FoldingNode* fn);
};
