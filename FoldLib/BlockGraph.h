#pragma once

#include "FdGraph.h"
#include "ShapeSuperKeyframe.h"
#include "FoldOptionGraph.h"


class ChainGraph;

class BlockGraph : public FdGraph
{
public:
	BlockGraph(QString id, Geom::Box shape_aabb);
	~BlockGraph();

	// selection
	ChainGraph* getSelChain();
	FdGraph* activeScaffold();
	void selectChain(QString id);
	QStringList getChainLabels();

	// top masters decide the folding duration 
	double	getNbTopMasters();

	// key frame
	virtual FdGraph* getKeyframe(double t, bool useThk) = 0;
	FdGraph* getSuperKeyframe(double t);

	// Folding regions and volumes
	virtual void computeAvailFoldingRegion(ShapeSuperKeyframe* ssKeyframe) = 0;
	virtual double getAvailFoldingVolume() = 0;
	QVector<QString> getInbetweenExternalParts(Vector3 base_center, Vector3 top_center, ShapeSuperKeyframe* ssKeyframe);

	// foldem
	virtual void foldabilize(ShapeSuperKeyframe* ssKeyframe) = 0;
	virtual void exportCollFOG();

	// apply fold solution with idx 
	virtual void applySolution(int idx) = 0;

	// thickness
	void setThickness(double thk);
public:
	// chains
	int selChainIdx;
	QVector<ChainGraph*> chains;

	// AABB of entire shape
	Geom::Box shapeAABB;

	// the base master
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;

	// time
	double timeScale; 
	Interval mFoldDuration;

	// upper bound for modification
	int nbSplits;
	int nbChunks;

	// trade-off weight for computing cost
	double weight;

	// available folding region
	bool ableToFold; // availFR >= minFR
	QVector<Geom::Rectangle2> availFoldingRegion;

	// fold solutions
	int selSlnIdx;
	QVector<QVector<FoldOption*> > foldSolutions;

	// the block has been foldabilized and ready to fold
	bool foldabilized;			

	// thickness
	bool useThickness;
	double thickness;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
}; 

