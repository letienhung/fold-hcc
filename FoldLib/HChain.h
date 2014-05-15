#pragma once

#include "ChainGraph.h"
#include "Rectangle2.h"

class HChain : public ChainGraph
{
public:
    HChain(FdNode* slave, PatchNode* master_low, PatchNode* master_high);

	// foldem
	QVector<FoldOption*> generateFoldOptions();

	// fold region
	Geom::Rectangle getFoldRegion(FoldOption* fn);
	Geom::Rectangle getMaxFoldRegion(bool right);

	// modifier
	QVector<Geom::Plane> generateCutPlanes( int N );
};

