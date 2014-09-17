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

	// #top masters: decides the folding duration 
	double	getNbTopMasters();

	// all fold options
	void genAllFoldOptions();

	// thickness
	void setThickness(double thk);

	// helper
	QVector<QString> getInbetweenExternalParts(Vector3 base_center, Vector3 top_center, ShapeSuperKeyframe* ssKeyframe);

public:
	//*** CORE
	// key frame
	virtual FdGraph*	getKeyframe(double t, bool useThk) = 0; // intermediate config. at local time t
	FdGraph*			getSuperKeyframe(double t);	// key frame with super master that merges all collapsed masters

	// foldabilization
	double						foldabilizeWrt(ShapeSuperKeyframe* ssKeyframe); // foldabilize a block wrt. the context and returns the cost
	virtual QVector<Vector2>	getObstaclePoints(ShapeSuperKeyframe* ssKeyframe) = 0; // get obstacles projected on the base master
	QVector<int>				getAvailFoldOptions(ShapeSuperKeyframe* ssKeyframe); // prune fold options wrt. to obstacles
	int							searchForExistedSolution(const QVector<int>& afo); // search for existed solution 
	virtual double				findOptimalSolution(const QVector<int>& afo) = 0; // store the optimal solution and returns the cost

	// apply fold solution
	virtual void applySolution(int idx) = 0;

	// debug
	virtual void exportCollFOG();

public:
	//*** ENTITIES
	// chains
	int selChainIdx;
	QVector<ChainGraph*> chains;

	// masters
	PatchNode* baseMaster;
	QVector<PatchNode*> masters;

public:
	//*** PARAMETERS
	// AABB of entire shape
	Geom::Box shapeAABB;

	// time
	double timeScale; 
	Interval mFoldDuration;

	// upper bound for modification
	int nbSplits;
	int nbChunks;

	// trade-off weight for computing cost
	double weight;

	// thickness
	bool useThickness;
	double thickness;

public:
	//*** SOLUTIONS
	// all fold options
	QVector<FoldOption*> allFoldOptions;

	// sets of fold options that have been foldabilized
	QVector< QVector<int> > testedAvailFoldOptions;
	QVector< QVector<int> > foldSolutions;
	QVector< double > foldCost;

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
}; 

