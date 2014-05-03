#pragma once

#include "ChainGraph.h"
#include "SectorCylinder.h"

class TChain : public ChainGraph
{
public:
    TChain(PatchNode* master, FdNode* slave);

	// foldem
	QVector<FoldOption*> generateFoldOptions();

	void applyFoldOption(FoldOption* fn);

	Geom::SectorCylinder getFoldingVolume(FoldOption* fn);

	void resolveCollision(FoldOption* fn);
};
