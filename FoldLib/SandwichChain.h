#pragma once

#include "ChainGraph.h"
#include "Rectangle2.h"

class SandwichChain : public ChainGraph
{
public:
    SandwichChain(FdNode* part, PatchNode* panel1, PatchNode* panel2);

	// fold options
	QVector<FoldingNode*> generateFoldOptions();
	void modify(FoldingNode* fn);

	// fold region on master patch
	Geom::Rectangle2 getFoldRegion(FoldingNode* fn);


	Geom::Segment2 getFoldingAxis2D(FoldingNode* fn);

	Geom::Segment getJointSegment(FoldingNode* fn);
};

