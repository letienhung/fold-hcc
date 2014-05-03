#pragma once
#include "BlockGraph.h"

class HBlock : public BlockGraph
{
public:
    HBlock(QVector<PatchNode*>& masters, QVector<FdNode*>& slaves, QVector< QVector<int> >& masterPairs, QString id);

	// foldem
	QVector<FoldOption*> generateFoldOptions();
	void applyFoldOption(FoldOption* fn);
	void foldabilize();
	void buildCollisionGraph();

	// animation
	QVector<Structure::Node*> getKeyFrameNodes( double t );
	
public:
	QVector<FoldOption*> foldSolution;
};

