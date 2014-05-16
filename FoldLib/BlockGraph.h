#pragma once

#include "FdGraph.h"
#include "FoldOptionGraph.h"

class HChain;

class BlockGraph : public FdGraph
{
public:
	BlockGraph(QVector<PatchNode*>& ms, QVector<FdNode*>& ss, 
		QVector< QVector<QString> >& mPairs, QString id);
	~BlockGraph();

	// selection
	HChain* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	// getter
	void exportCollFOG();
	double getTimeLength();

	// keyframes
	/** a stand-alone scaffold representing the folded block at given time
		this scaffold can be requested for position of folded master and slave parts
		this scaffold need be translated to combine with key frame scaffold from other blocks
		to form the final folded scaffold
		this scaffold has to be deleted by whoever calls this function
	**/
	FdGraph* getKeyframeScaffold(double t);

	// folding volumes
	void computeMinFoldingVolume();
	void computeMaxFoldingVolume(Geom::Box cropper);
	QMap<QString, Geom::Box> computeAvailFoldingVolume(FdGraph* scaffold, 
									QMultiMap<QString, QString>& masterOrderConstraints);

	// foldem
	void foldabilize();
	void buildCollisionGraph();
	void findOptimalSolution();
	bool fAreasIntersect(Geom::Rectangle& rect1, Geom::Rectangle& rect2);

	// helper
	QVector<QString> getInbetweenParts(FdGraph* scaffold, QString mid1, QString mid2);

public:
	// time interval
	TimeInterval mFoldDuration;

	// master related
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
	QMap<QString, QSet<int> > masterChainsMap; 
	QMap<QString, QSet<int> > masterUnderChainsMap; // master : chains under master
	QMap<QString, double> masterTimeStamps;

	// chains
	int selChainIdx;
	QVector<HChain*> chains;

	// folding volumes
	QMap<QString, Geom::Rectangle> minFoldingRegion;
	QMap<QString, Geom::Box> minFoldingVolume;
	QMap<QString, Geom::Box> maxFoldingVolume;
	QMap<QString, Geom::Box> availFoldingVolume;

	// collision graph
	FoldOptionGraph* collFog;
	FoldOptionGraph* collFogOrig;
	QVector<FoldOption*> foldSolution;
}; 

