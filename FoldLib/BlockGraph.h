#pragma once

#include "FdGraph.h"
#include "FoldOptionGraph.h"

class ChainGraph;

class BlockGraph : public FdGraph
{
public:
	BlockGraph(QString id, QVector<PatchNode*>& ms, QVector<FdNode*>& ss, 
		QVector< QVector<QString> >& mPairs, Geom::Box shape_aabb );
	~BlockGraph();

	// selection
	ChainGraph* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	// key frame
	FdGraph* getKeyframe(double t);
	FdGraph* getSuperKeyframe(double t);
	void computeSuperBlock(FdGraph* superKeyframe);

	// folding space
	void computeMinFoldingRegion();
	void computeMaxFoldingRegion();
	void computeAvailFoldingRegion(FdGraph* scaffold);
	double getAvailFoldingVolume();
	Geom::Box getAvailFoldingSpace(QString mid);
	QVector<Geom::Box> getAFS();
	QVector<QString> getInbetweenOutsideParts(FdGraph* superKeyframe, QString mid1, QString mid2);
	QVector<QString> getUnrelatedMasters(FdGraph* superKeyframe, QString mid1, QString mid2);

	// collision graph
	void addNodesToCollisionGraph();
	void filterFoldOptions(QVector<FoldOption*>& options, int cid);
	void addEdgesToCollisionGraph();
	void exportCollFOG();
	void setNbSplits(int N);
	void setNbChunks(int N);

	// thickness
	void setThickness(double thk);
	void setUseThickness(bool use);
	void updateSolutionWithThickness();
	void computeMasterNbUnderLayers();

	// foldem
	void foldabilize(FdGraph* superKeyframe);
	void findOptimalSolution();
	bool fAreasIntersect(Geom::Rectangle& rect1, Geom::Rectangle& rect2);

	// apply fold solution
	void applySolution(int sid);

	// helper
	double getTimeLength();
public:
	// time interval
	Interval mFoldDuration;

	// master related
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
	QMap<QString, double> masterHeight;             
	QMap<QString, QSet<int> > masterChainsMap; 
	QMap<QString, QSet<int> > masterUnderChainsMap; // master : chains under master
	QMap<int, QString> chainBaseMasterMap;

	// time stamps
	double timeScale;
	QMap<QString, double> masterTimeStamps;
	QVector<QString> sortedMasters; // bottom-up

	// chains
	int selChainIdx;
	QVector<ChainGraph*> chains;

	// AABB of entire shape
	Geom::Box shapeAABB;

	// collision graph
	int nbSplits;
	int nbChunks;
	FoldOptionGraph* collFog;
	QVector<FoldOptionGraph*> debugFogs;

	// fold solutions
	int selSlnIdx;
	QVector<QVector<FoldOption*> > foldSolutions;

	// super block
	QMap<QString, QString> master2Super;
	FdGraph* superBlock;
	PatchNode* baseMasterSuper;
	QVector<PatchNode*> mastersSuper;
	QMap<QString, double> masterHeightSuper;    
	QMap<QString, QSet<int> > masterUnderChainsMapSuper;
	QMap<int, QString> chainTopMasterMapSuper;

	// folding regions
	// ***2D rectangles encoded in original base patch
	QMap<QString, Geom::Rectangle2> minFoldingRegion;
	QMap<QString, Geom::Rectangle2> maxFoldingRegion;
	QMap<QString, Geom::Rectangle2> availFoldingRegion;

	// thickness
	bool useThickness;
	double thickness;
	QMap<QString, int> masterNbUnderLayers;
}; 

