#pragma once
#include "BlockGraph.h"

class HBlock : public BlockGraph
{
public:
    HBlock(QVector<FdNode*> parts, PatchNode* panel1, PatchNode* panel2, QString id, Geom::Box &bBox);

	// collision graph
	void foldabilize();
	void buildCollisionGraph();

	QVector<Structure::Node*> getKeyFrameNodes( double t );

public:
	PatchNode *mPanel1, *mPanel2;
	QVector<FoldingNode*> foldSolution;
};

