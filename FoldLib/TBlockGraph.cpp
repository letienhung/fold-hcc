#include "TBlockGraph.h"
#include "TChainGraph.h"
#include "Numeric.h"

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
	// generate fold options
	QVector<FoldOption*> options = tChain->genFoldOptions(nbSplits, nbChunks);

	// prune by AFS
	Geom::Rectangle base_rect = baseMaster->mPatch;
	Geom::Rectangle2 AFR = getAvailFR(ableToFoldRight);
	AFR.Extent *= 1.05; // to avoid numerical issue
	QVector<FoldOption*> valid_options;
	foreach(FoldOption* fn, options)
	{
		Geom::Rectangle2 fRegion2 = base_rect.get2DRectangle(fn->region);
		if (AFR.containsAll(fRegion2.getConners()))
			valid_options << fn;
	}

	// choose the one with the lowest cost
	FoldOption* best_fn;
	double minCost = maxDouble();
	foreach(FoldOption* fn, valid_options){
		double cost = fn->getCost(weight);
		if (cost < minCost)
		{
			best_fn = fn;
			minCost = cost;
		}
	}

	// apply fold option
	tChain->applyFoldOption(best_fn);

	// tag
	foldabilized = true;

	// debug
	properties[FOLD_REGIONS].setValue(QVector<Geom::Rectangle>() << best_fn->region);
}

FdGraph* TBlockGraph::getKeyframe(double t, bool useThk)
{
	FdGraph* keyframe = nullptr;

	// chains have been created and ready to fold
	// IOW, the block has been foldabilized
	if (foldabilized)
	{
		keyframe = tChain->getKeyframe(t, useThk);
	}
	// the block is not ready
	// can only answer request on t = 0 and t = 1
	else
	{
		// t = 0 : the entire block
		if (t < 0.5)
			keyframe = (FdGraph*)tChain->clone();
		else
		// t = 1 : just the base master
		{
			keyframe = new FdGraph();
			keyframe->Structure::Graph::addNode(baseMaster->clone());
		}
	}

	return keyframe;
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

	// check if minFR is available
	Geom::Rectangle2 avail_region = getMinFR(isRight);
	bool okay = !avail_region.containsAny(sample_proj, -0.1);
	getAbleToFoldTag(isRight) = okay;
	if (!okay) return;

	// expand
	avail_region.expandToTouch(sample_proj, -0.1);
	getAvailFR(isRight) = avail_region;

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