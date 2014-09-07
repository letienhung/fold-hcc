#include "TBlockGraph.h"

TBlockGraph::TBlockGraph(QString id, QVector<PatchNode*>& ms, QVector<FdNode*>& ss,
	QVector< QVector<QString> >& mPairs, Geom::Box shape_aabb)
	:BlockGraph(id, shape_aabb)
{

}

void TBlockGraph::foldabilize(ShapeSuperKeyframe* ssKeyframe)
{
	//// the chain
	//ChainGraph* chain = chains.front();

	//// generate T-options
	//QVector<FoldOption*> options;
	//for (int nS = 1; nS <= nbSplits; nS++)
	//for (int nUsedChunks = nbChunks; nUsedChunks >= 1; nUsedChunks--)
	//	options << chain->genFoldOptions(nS, nUsedChunks, nbChunks);

	//// prune by AFS
	//Geom::Rectangle base_rect = baseMaster->mPatch;
	//QString top_mid_super = superBlock->chainTopMasterMap[0];
	//Geom::Rectangle2 AFR = superBlock->availFoldingRegion[top_mid_super];
	//AFR.Extent *= 1.01; // ugly way to avoid numerical issue
	//QVector<FoldOption*> valid_options;
	//foreach(FoldOption* fn, options)
	//{
	//	Geom::Rectangle2 fRegion2 = base_rect.get2DRectangle(fn->region);
	//	if (AFR.containsAll(fRegion2.getConners()))
	//		valid_options << fn;
	//}

	//// choose the one with the lowest cost
	//FoldOption* best_fn;
	//double minCost = maxDouble();
	//foreach(FoldOption* fn, valid_options){
	//	double cost = computeCost(fn);
	//	if (cost < minCost)
	//	{
	//		best_fn = fn;
	//		minCost = cost;
	//	}
	//}
	//best_fn->addTag(SELECTED_TAG);

	//// apply fold option
	//chain->applyFoldOption(best_fn);
	//foldabilized = true;
}

FdGraph* TBlockGraph::getKeyframe(double t, bool useThk)
{
	return nullptr;
}

void TBlockGraph::computeAvailFoldingRegion(ShapeSuperKeyframe* ssKeyframe)
{

}

double TBlockGraph::getAvailFoldingVolume()
{
	return 0;
}

void TBlockGraph::applySolution(int idx)
{
	Q_UNUSED(idx);
}
