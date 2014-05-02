#pragma once

#include "ChainGraph.h"
#include "SectorCylinder.h"

class TChain : public ChainGraph
{
public:
    TChain(PatchNode* master, FdNode* slave);

	QVector<FoldOption*> generateFoldOptions();
	void modify(FoldOption* fn);

	Geom::SectorCylinder getFoldingVolume(FoldOption* fn);

	void resolveCollision(FoldOption* fn);
};
