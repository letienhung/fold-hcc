#pragma once

#include "ChainGraph.h"
#include "FoldOptionGraph.h"

class BlockGraph : public FdGraph
{
public:
	BlockGraph(QVector<PatchNode*>& masters, QVector<FdNode*>& slaves, 
		QVector< QVector<QString> >& masterPairs, QString id);
	~BlockGraph();

	// assign master time stamps
	void assignMasterTimeStamps();

	// selection
	ChainGraph* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	// getter
	void exportCollFOG();

	// keyframes
	/** a stand-alone scaffold representing the folded block at given time
		this scaffold can be requested for position of folded master and slave parts
		this scaffold need be translated to combine with key frame scaffold from other blocks
		to form the final folded scaffold
		this scaffold has to be deleted by whoever calls this function
	**/
	FdGraph* getKeyframeScaffold(double t);

	// folding volumes
	void computeBFV();

	// foldem
	void foldabilize();
	void buildCollisionGraph();
	void findOptimalSolution();
	bool fAreasIntersect(Geom::Rectangle& rect1, Geom::Rectangle& rect2);

public:
	// time interval
	TimeInterval mFoldDuration;

	// master related
	PatchNode* baseMaster;
	QVector<PatchNode*> mMasters;
	QMap<QString, QSet<int> > masterChainsMap;
	QMap<QString, double> masterTimeStamps;

	// chains
	int selChainIdx;
	QVector<ChainGraph*> mChains;

	// folding volumes
	QMap<QString, Geom::Box> basicFoldingVolume;
	QMap<QString, Geom::Box> extendedFoldingVolume;

	// collision graph
	FoldOptionGraph* collFog;
	FoldOptionGraph* collFogOrig;
	QVector<FoldOption*> foldSolution;
}; 

