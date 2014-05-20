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
	void updateCollisionLinks();
	void buildCollisionGraph();
	void buildCollisionGraphAdaptive();
	int encodeModification(int nX, int nY);
	void decodeModification(int mdf, int& nX, int& nY);
	void genNewModifications(QSet<int>& modifications, int max_nX, int nChunks);
	void filterFoldOptions(QVector<FoldOption*>& options, int cid);
	void findOptimalSolution();
	bool fAreasIntersect(Geom::Rectangle& rect1, Geom::Rectangle& rect2);

	// apply fold solution
	void applySolution(int sid);

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
	QVector<FoldOptionGraph*> debugFogs;

	// fold solutions
	QVector<QVector<FoldOption*> > foldSolutions;
}; 

