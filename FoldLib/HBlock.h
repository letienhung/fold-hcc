#pragma once
#include "BlockGraph.h"
#include "FoldOptionGraph.h"

class HBlock : public BlockGraph
{
public:
    HBlock(QVector<PatchNode*>& masters, QVector<FdNode*>& slaves, 
		QVector< QVector<QString> >& masterPairs, Geom::Box bb, QString id);

	// assign master time stamps
	void assignMasterTimeStamps();

	// foldem
	QVector<FoldOption*> generateFoldOptions();
	QVector<Geom::Box> getFoldVolume();
	void applyFoldOption(FoldOption* fn);
	void foldabilize();
	void buildCollisionGraph();

	// key frames
	FdGraph* getKeyframeScaffold( double t );

	// helper
	bool fAreasIntersect(Geom::Rectangle& rect1, Geom::Rectangle& rect2);
	
	// getter
	double getTimeLength();

public:
	QMap<QString, QSet<int> > masterChainsMap;
	QMap<QString, double> masterTimeStamps;

	FoldOptionGraph* collFog;
	QVector<FoldOption*> foldSolution;
};

