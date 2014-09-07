#pragma once

#include "BlockGraph.h"

#include "FoldOptionGraph.h"
#include "SuperBlockGraph.h"

class HBlockGraph : public BlockGraph
{
public:
	HBlockGraph(QString id, QVector<PatchNode*>& ms, QVector<FdNode*>& ss,
		QVector< QVector<QString> >& mPairs, Geom::Box shape_aabb);
	~HBlockGraph();

	// H chains
	void createChains(QVector<FdNode*>& ss, QVector< QVector<QString> >& mPairs);

	// key frame
	virtual FdGraph* getKeyframe(double t, bool useThk) override;

	// foldem
	virtual void foldabilize(ShapeSuperKeyframe* ssKeyframe) override;
	void findOptimalSolution();

	// collision graph
	void addNodesToCollisionGraph();
	void pruneFoldOptions(QVector<FoldOption*>& options, int cid);
	void addEdgesToCollisionGraph();
	virtual void exportCollFOG() override;

	// apply solution
	virtual void applySolution(int idx) override;

	// available folding region
	virtual void computeAvailFoldingRegion(ShapeSuperKeyframe* ssKeyframe) override;
	virtual double getAvailFoldingVolume() override;

	// helper
	bool fAreasIntersect(Geom::Rectangle& rect1, Geom::Rectangle& rect2);

public:
	// master related
	QMap<QString, double> masterHeight;
	QMap<QString, QSet<int> > masterChainsMap;
	QMap<QString, QSet<int> > masterUnderChainsMap; // master : chains under master
	QMap<int, QString> chainBaseMasterMap;

	// timing 
	QVector<QString> sortedMasters; // bottom-up
	QMap<QString, double> masterTimeStamps;

	// collision graph
	FoldOptionGraph* collFog;

	// super block
	SuperBlockGraph* superBlock;
};

