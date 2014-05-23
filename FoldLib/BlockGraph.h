#pragma once

#include "FdGraph.h"
#include "FoldOptionGraph.h"

class HChain;

class BlockGraph : public FdGraph
{
public:
	BlockGraph(QString id, QVector<PatchNode*>& ms, QVector<FdNode*>& ss, 
		QVector< QVector<QString> >& mPairs, Geom::Box shape_aabb );
	~BlockGraph();

	// selection
	HChain* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	// getter
	void exportCollFOG();
	double getTimeLength();

	// key frame
	/** a stand-alone scaffold representing the folded block at given time
		this scaffold can be requested for position of folded master and slave parts
		this scaffold need be translated to combine with key frame scaffold from other blocks
		to form the final folded scaffold
		this scaffold has to be deleted by whoever calls this function
	**/
	FdGraph* getKeyframe(double t);
	FdGraph* getSuperKeyframe(double t);

	// super block
	void computeSuperBlock(FdGraph* superKeyframe);

	// folding space
	void computeMinFoldingRegion();
	void computeMaxFoldingRegion();
	void computeAvailFoldingRegion(FdGraph* scaffold);
	double getAvailFoldingVolume();
	Geom::Box getAvailFoldingSpace(QString mid);
	QVector<Geom::Box> getAFS();

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
	QVector<QString> getInbetweenOutsideParts(FdGraph* superKeyframe, QString mid1, QString mid2);
	QVector<QString> getUnrelatedMasters(FdGraph* superKeyframe, QString mid1, QString mid2);
public:
	// time interval
	TimeInterval mFoldDuration;

	// master related
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
	QMap<QString, double> masterHeight;             
	QMap<QString, QSet<int> > masterChainsMap; 
	QMap<QString, QSet<int> > masterUnderChainsMap; // master : chains under master

	// time stamps
	double timeScale;
	QMap<QString, double> masterTimeStamps;

	// chains
	int selChainIdx;
	QVector<HChain*> chains;

	// AABB of entire shape
	Geom::Box shapeAABB;

	// collision graph
	FoldOptionGraph* collFog;
	QVector<FoldOptionGraph*> debugFogs;

	// fold solutions
	QVector<QVector<FoldOption*> > foldSolutions;

	// super block
	FdGraph* superBlock;
	PatchNode* baseMasterSuper;
	QVector<PatchNode*> mastersSuper;
	QMap<QString, double> masterHeightSuper;    
	QMap<QString, QSet<int> > masterUnderChainsMapSuper;

	// folding regions
	// ***2D rectangles encoded in original base patch
	QMap<QString, Geom::Rectangle2> minFoldingRegion;
	QMap<QString, Geom::Rectangle2> maxFoldingRegion;
	QMap<QString, Geom::Rectangle2> availFoldingRegion;
}; 

