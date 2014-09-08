#include "TBlockGraph.h"
#include "TChainGraph.h"

TBlockGraph::TBlockGraph(QString id, QVector<PatchNode*>& ms, QVector<FdNode*>& ss,
	QVector< QVector<QString> >& mPairs, Geom::Box shape_aabb)
	:BlockGraph(id, shape_aabb)
{
	// clone nodes
	foreach(PatchNode* m, ms)	{
		masters << (PatchNode*)m->clone();
		Structure::Graph::addNode(masters.last());
	}
	foreach(FdNode* s, ss)
		Structure::Graph::addNode(s->clone());

	// the base and virtual top masters
	bool isVirtualFirst = masters.front()->hasTag(EDGE_ROD_TAG);
	topMaster = isVirtualFirst? masters.front() : masters.last();
	baseMaster = isVirtualFirst ? masters.last() : masters.front();

	// the chain
	tChain = new TChainGraph(ss.front(), baseMaster, topMaster);
	tChain->setFoldDuration(0, 1);
	chains << tChain;

	// tag
	ableToFoldLeft = false;
	ableToFoldRight = false;
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
	return chains.front()->getKeyframe(t, useThk);
}


Geom::Rectangle2& TBlockGraph::getMinFR(bool isRight)
{
	return isRight ? minFoldingRegionRight : minFoldingRegionLeft;
}

Geom::Rectangle2& TBlockGraph::getMaxFR(bool isRight)
{
	return isRight ? maxFoldingRegionRight : maxFoldingRegionLeft;
}

Geom::Rectangle2& TBlockGraph::getAvailFR(bool isRight)
{
	return isRight ? availFoldingRegionRight : availFoldingRegionLeft;
}

bool& TBlockGraph::getAbleToFoldTag(bool isRight)
{
	return isRight ? ableToFoldRight : ableToFoldLeft;
}

double TBlockGraph::getHeight()
{
	return tChain->topTraj.length();
}


void TBlockGraph::computeAvailFoldingRegion(ShapeSuperKeyframe* ssKeyframe)
{
	// clear
	ableToFoldLeft = false;
	ableToFoldRight = false;
	properties[MINFR].setValue(QVector<Vector3>());
	properties[MAXFR].setValue(QVector<Vector3>());
	properties[AFR_CP].setValue(QVector<Vector3>());

	// compute
	computeAvailFoldingRegion(true, ssKeyframe);
	computeAvailFoldingRegion(false, ssKeyframe);

	// tag
	ableToFold = ableToFoldRight || ableToFoldLeft;
}

void TBlockGraph::computeAvailFoldingRegion(bool isRight, ShapeSuperKeyframe* ssKeyframe)
{
	// min and max FR
	computeMinFoldingRegion(isRight);
	computeMaxFoldingRegion(isRight);

	// constraints from in-between external parts
	QVector<Vector3> samples;
	QVector<Vector2> sample_proj;
	for (auto nid : getInbetweenExternalParts(baseMaster->center(), topMaster->center(), ssKeyframe))
		samples << ssKeyframe->getFdNode(nid)->sampleBoundabyOfScaffold(100);
	Geom::Rectangle base_rect = baseMaster->mPatch;
	for (auto s : samples) sample_proj << base_rect.getProjCoordinates(s);

	// sample boundary of max region
	sample_proj << getMaxFR(isRight).getEdgeSamples(100);

	// extend min to max
	Geom::Rectangle2 minFR = getMinFR(isRight);
	bool okay = extendRectangle2D(minFR, sample_proj);
	getAbleToFoldTag(isRight) = okay;
	if (!okay) return;

	// store avail FR
	getAvailFR(isRight) = minFR;

	// debug 
	appendToVectorProperty( AFR_CP, base_rect.get3DRectangle(getAvailFR(isRight)).getEdgeSamples(100) );
}

void TBlockGraph::computeMinFoldingRegion(bool isRight)
{
	// the minFR is the folding region with maximum number of splits
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Rectangle minFR3 = tChain->ChainGraph::getFoldRegion(isRight, nbSplits);
	getMinFR(isRight) = base_rect.get2DRectangle(minFR3);

	// debug
	appendToVectorProperty(MINFR, base_rect.get3DRectangle(getMinFR(isRight)).getEdgeSamples(25));
}

void TBlockGraph::computeMaxFoldingRegion(bool isRight)
{
	// the maxFR is the folding region with minimum number of splits
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Rectangle maxFR3 = tChain->ChainGraph::getFoldRegion(isRight, 0);
	getMaxFR(isRight) = base_rect.get2DRectangle(maxFR3);

	// maxFR is cropped by shape AABB
	int aid = shapeAABB.getAxisId(base_rect.Normal);
	Geom::Rectangle cropper3 = shapeAABB.getPatch(aid, 0);
	Geom::Rectangle2 cropper2 = base_rect.get2DRectangle(cropper3);
	getMaxFR(isRight).cropByAxisAlignedRectangle(cropper2);

	// debug
	appendToVectorProperty(MAXFR, base_rect.get3DRectangle(getMaxFR(isRight)).getEdgeSamples(50));
}

double TBlockGraph::getAvailFoldingVolume()
{
	// AFS is empty
	if (!ableToFold) return 0;

	// choose the side that is available
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Rectangle2 afr = getAvailFR(ableToFoldRight);
	Geom::Rectangle afr3 = base_rect.get3DRectangle(afr);
	return afr3.area() * getHeight();
}


void TBlockGraph::applySolution(int idx)
{
	Q_UNUSED(idx);
}