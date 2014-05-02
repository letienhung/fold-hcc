#pragma once

#include "ChainGraph.h"
#include "Rectangle2.h"

class HChain : public ChainGraph
{
public:
    HChain(FdNode* slave, PatchNode* master1, PatchNode* master2);

	// fold options
	QVector<FoldingNode*> generateFoldOptions();
	void modify(FoldingNode* fn);

	// fold region on master patch
	Geom::Rectangle2 getFoldRegion(FoldingNode* fn);


	Geom::Segment2 getFoldingAxis2D(FoldingNode* fn);

	Geom::Segment getJointSegment(FoldingNode* fn);
};

