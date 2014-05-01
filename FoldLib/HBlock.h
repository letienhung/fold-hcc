#pragma once
#include "BlockGraph.h"

class HBlock : public BlockGraph
{
public:
    HBlock(QVector<PatchNode*> masters, QVector<FdNode*> slaves, const QVector< QSet<int> >& slaveEnds, QString id);

	// collision graph
	void foldabilize();
	void buildCollisionGraph();

	QVector<Structure::Node*> getKeyFrameNodes( double t );

public:
	QVector<FoldingNode*> foldSolution;
};

