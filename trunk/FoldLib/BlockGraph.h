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

	// folding space
	void computeMinFoldingRegion();
	void computeMaxFoldingRegion(Geom::Box cropper);
	void computeAvailFoldingRegion(FdGraph* scaffold, 
		QMultiMap<QString, QString>& moc_greater, QMultiMap<QString, QString>& moc_less);
	double getAvailFoldingVolume();
	Geom::Box getAvailFoldingSpace(QString mid);

	// foldem
	void foldabilize();
	void buildCollisionGraph();
	void findOptimalSolution();
	bool fAreasIntersect(Geom::Rectangle& rect1, Geom::Rectangle& rect2);

	// helper
	QVector<QString> getInbetweenOutsideParts(FdGraph* scaffold, QString mid1, QString mid2);

public:
	// time interval
	TimeInterval mFoldDuration;

	// master related
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
	QMap<QString, QSet<int> > masterChainsMap; 
	QMap<QString, QSet<int> > masterUnderChainsMap; // master : chains under master

	// time stamps
	double timeScale;
	QMap<QString, double> masterTimeStamps;

	// chains
	int selChainIdx;
	QVector<HChain*> chains;

	// folding space
	QMap<QString, double> masterHeight;
	QMap<QString, Geom::Rectangle2> minFoldingRegion;
	QMap<QString, Geom::Rectangle2> maxFoldingRegion;
	QMap<QString, Geom::Rectangle2> availFoldingRegion;

	// collision graph
	FoldOptionGraph* collFog;
	FoldOptionGraph* collFogOrig;
	QVector<FoldOption*> foldSolution;
}; 
