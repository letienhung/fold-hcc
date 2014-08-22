#pragma once

#include "FdGraph.h"
#include "FoldOptionGraph.h"
#include "ShapeSuperKeyframe.h"

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
	FdGraph* getKeyframe(double t, bool useThk);
	FdGraph* getSuperKeyframe(double t);
	void computeSuperBlock(ShapeSuperKeyframe* ssKeyframe);

	// folding space
	void computeMinFoldingRegion();
	void computeMaxFoldingRegion();
	void computeAvailFoldingRegion(ShapeSuperKeyframe* ssKeyframe);
	double getAvailFoldingVolume();
	Geom::Box getAvailFoldingSpace(QString mid);
	QVector<Geom::Box> getAFS();
	QVector<QString> getInbetweenExternalParts(ShapeSuperKeyframe* ssKeyframe, QString mid1, QString mid2);
	QVector<QString> getUnrelatedMasters(ShapeSuperKeyframe* ssKeyframe, QString mid1, QString mid2);

	// collision graph
	void addNodesToCollisionGraph();
	void filterFoldOptions(QVector<FoldOption*>& options, int cid);
	void addEdgesToCollisionGraph();
	void exportCollFOG();

	// foldem
	void foldabilize(ShapeSuperKeyframe* ssKeyframe);
	void findOptimalSolution();

	// T-block
	bool isTBlock();
	void foldabilizeTBlock();

	// apply fold solution
	void applySolution(int sid);

	// helper
	double	getTimeLength();
	double	computeCost(FoldOption* fn);
	bool	fAreasIntersect(Geom::Rectangle& rect1, Geom::Rectangle& rect2);

	// thickness
	void setThickness(double thk);
	void computeMasterNbUnderLayers();

public:
	// AABB of entire shape
	Geom::Box shapeAABB;

	// chains
	int selChainIdx;
	QVector<ChainGraph*> chains;

	// master related
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;
	QMap<QString, double> masterHeight;             
	QMap<QString, QSet<int> > masterChainsMap; 
	QMap<QString, QSet<int> > masterUnderChainsMap; // master : chains under master
	QMap<int, QString> chainBaseMasterMap;

	// time
	double timeScale; 
	Interval mFoldDuration;
	QVector<QString> sortedMasters; // bottom-up
	QMap<QString, double> masterTimeStamps;

	// collision graph
	int nbSplits;
	int nbChunks;
	FoldOptionGraph* collFog;

	// fold solutions
	int selSlnIdx;
	bool foldabilized;			// the block has been foldabilized and ready to fold
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

	// cost weight
	double w;

	// single block
	bool isAlone;
}; 

