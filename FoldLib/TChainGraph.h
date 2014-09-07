#pragma once

#include "ChainGraph.h"

class TChainGraph : public ChainGraph
{
public:
	TChainGraph(FdNode* slave, PatchNode* base, PatchNode* top);

	// override functions
	virtual Geom::Rectangle getFoldRegion(FoldOption* fn) override;

	// cut planes
	virtual QVector<Geom::Plane> generateCutPlanes(FoldOption* fn) override;

	// fold
	virtual void fold(double t) override;
};
