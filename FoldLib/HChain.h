#pragma once

#include "ChainGraph.h"
#include "Rectangle2.h"

class HChain : public ChainGraph
{
public:
    HChain(FdNode* slave, PatchNode* master_low, PatchNode* master_high);

	// foldem
	// nX: splits; nY: shrink
	QVector<FoldOption*> generateFoldOptions(int nSplits, int nUsedChunks, int nChunks);

	// fold region
	Geom::Rectangle getFoldRegion(FoldOption* fn);
	Geom::Rectangle getMaxFoldRegion(bool right);

	// top
	PatchNode* getTopMaster();

	// modifier
	QVector<Geom::Plane> generateCutPlanes( int N );
};

