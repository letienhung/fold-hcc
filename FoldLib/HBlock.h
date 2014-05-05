#pragma once
#include "BlockGraph.h"
#include "FoldOptionGraph.h"

class HBlock : public BlockGraph
{
public:
    HBlock(QVector<PatchNode*>& masters, QVector<FdNode*>& slaves, 
		QVector< QVector<QString> >& masterPairs, QString id);

	// assign master time stamps
	void assignMasterTimeStamps();

	// foldem
	QVector<FoldOption*> generateFoldOptions();
	void applyFoldOption(FoldOption* fn);
	void foldabilize();
	void buildCollisionGraph();

	// key frames
	FdGraph* getKeyframeScaffold( double t );
	
	// getter
	int nbTimeUnits();

public:
	QString baseMid;
	QMap<QString, QSet<int> > masterChainsMap;
	QMap<QString, double> masterTimeStamps;
	QVector<TimeInterval> chainTimeIntervals;

	FoldOptionGraph* collFog;
	QVector<FoldOption*> foldSolution;
};

