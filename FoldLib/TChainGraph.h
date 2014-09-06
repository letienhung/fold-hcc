#pragma once

#include "ChainGraph.h"

class TChainGraph : public ChainGraph
{
public:
	TChainGraph(FdNode* slave, PatchNode* base, PatchNode* top);

	// override functions
	Geom::Rectangle getFoldRegion(FoldOption* fn);
	Geom::Rectangle getMaxFoldRegion(bool isRight);
};
