#pragma once

#include "ChainGraph.h"
#include "Rectangle2.h"

class HChain : public ChainGraph
{
public:
    HChain(FdNode* slave, PatchNode* master1, PatchNode* master2);

	// foldem
	QVector<FoldOption*> generateFoldOptions();
	void applyFoldOption(FoldOption* fn);

	// fold region
	Geom::Rectangle2 getFoldRegion(FoldOption* fn);

	// modifier
	QVector<Geom::Plane> generateCutPlanes( int N );
};

