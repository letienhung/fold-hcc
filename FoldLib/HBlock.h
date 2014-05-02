#pragma once
#include "BlockGraph.h"

class HBlock : public BlockGraph
{
public:
    HBlock(QVector<PatchNode*>& masters, QVector<FdNode*>& slaves, QVector< QVector<int> >& masterPairs, QString id);

	// collision graph
	void foldabilize();
	void buildCollisionGraph();

	QVector<Structure::Node*> getKeyFrameNodes( double t );
	
public:
	// time interval
	double start, end;

	QVector<FoldingNode*> foldSolution;
};

