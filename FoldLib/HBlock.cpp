#include "HBlock.h"
#include "HChain.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "IntrRect2Rect2.h"
#include "Numeric.h"
#include "CliquerAdapter.h"

HBlock::HBlock( QVector<PatchNode*>& masters, QVector<FdNode*>& slaves,  
				QVector< QVector<QString> >& masterPairs, QString id )
				:BlockGraph(id)
{
	// type
	mType = BlockGraph::H_BLOCK;

	// clone nodes
	foreach (PatchNode* m, masters) 
		Structure::Graph::addNode(m->clone());
	foreach (FdNode* s, slaves) 
		Structure::Graph::addNode(s->clone());

	// base master
	baseMasterId = masters.front()->mID;

	// sort masters
	assignMasterTimeStamps();

	// create chains
	for (int i = 0; i < slaves.size(); i++)
	{
		QString mid1 = masterPairs[i].front();
		QString mid2 = masterPairs[i].last();
		
		// create chain
		HChain* hc = new HChain(slaves[i], (PatchNode*)getNode(mid1), (PatchNode*)getNode(mid2));
		hc->mFoldDuration = TIME_INTERVAL(masterTimeStamps[mid1], masterTimeStamps[mid2]);
		chains << hc;

		// map from master id to chain idx set
		masterChainsMap[mid1] << i;
		masterChainsMap[mid2] << i;
	}

	// initial collision graph
	collFog = new FoldOptionGraph();
}

void HBlock::assignMasterTimeStamps()
{
	QVector<PatchNode*> masters;
	foreach(Structure::Node* n, getNodesWithTag(IS_MASTER))
		masters << (PatchNode*)n;

	// squeezing line
	Geom::Rectangle rect0 = masters[0]->mPatch;
	Geom::Line squzLine(rect0.Center, rect0.Normal);

	// projected position along squeezing line
	double minT = maxDouble();
	double maxT = -maxDouble();
	foreach (PatchNode* m, masters)
	{
		double t = squzLine.getProjTime(m->center());
		masterTimeStamps[m->mID] = t;

		if (t < minT) minT = t;
		if (t > maxT) maxT = t;
	}

	// normalize time stamps
	double timeRange = maxT - minT;
	foreach (QString mid, masterTimeStamps.keys())
	{
		masterTimeStamps[mid] = ( masterTimeStamps[mid] - minT ) / timeRange;
	}
}


void HBlock::foldabilize()
{
	buildCollisionGraph();

	// get all folding nodes
	QVector<FoldOption*> fns = collFog->getAllFoldOptions();

	// the dual adjacent matrix
	QVector<bool> dumpy(fns.size(), true);
	QVector< QVector<bool> > conn(fns.size(), dumpy);
	for (int i = 0; i < fns.size(); i++){
		// the i-th node
		FoldOption* fn = fns[i];

		// diagonal entry
		conn[i][i] = false;

		// other fold options
		for (int j = i+1; j < fns.size(); j++){
			// the j-th node
			FoldOption* other_fn = fns[j];

			// connect siblings and colliding folding options
			if (collFog->areSiblings(fn->mID, other_fn->mID) ||
				collFog->verifyLinkType(fn->mID, other_fn->mID, "collision")){
				conn[i][j] = false;	conn[j][i] = false;
			}
		}
	}

	// find minimum cost maximum cliques
	QVector<double> weights;
	foreach(FoldOption* fn, fns) weights.push_back(fn->getCost());
	CliquerAdapter cliquer(conn, weights);
	cliquer.computeWeightsOfAllMaxCliques();
	QVector<int> q = cliquer.getMinWeightMaxClique();

	// fold solution
	foldSolution.clear();
	foldSolution.resize(chains.size());
	foreach(int idx, q)
	{
		FoldOption* fn = fns[idx];
		FoldEntity* cn = collFog->getFoldEntity(fn->mID);
		foldSolution[cn->entityIdx] = fn;
	}

	// apply fold options
	for (int i = 0; i < chains.size(); i++)
	{
		chains[i]->applyFoldOption(foldSolution[i]);
	}
}

void HBlock::buildCollisionGraph()
{
	// clear
	collFog->clear();

	// fold entities and options
	for(int i = 0; i < chains.size(); i++)
	{
		HChain* chain = (HChain*)chains[i];

		// fold entity
		FoldEntity* cn = new FoldEntity(i, chain->mID);
		collFog->addNode(cn);

		// fold options and links
		foreach(FoldOption* fn, chain->generateFoldOptions())
		{
			// to-do: reject if collide with other masters
			// whose time stamp is within the time interval of fn

			collFog->addNode(fn);
			collFog->addFoldLink(cn, fn);
		}
	}

	// collision links
	QVector<FoldOption*> fns = collFog->getAllFoldOptions();
	for (int i = 0; i < fns.size(); i++)
	{
		FoldOption* fn = fns[i];
		FoldEntity* cn = collFog->getFoldEntity(fn->mID);
		Geom::Rectangle2 fArea = fn->properties["fArea"].value<Geom::Rectangle2>();
		TimeInterval tInterval = chains[cn->entityIdx]->mFoldDuration;

		// with other fold options
		for (int j = i+1; j < fns.size(); j++)
		{
			FoldOption* other_fn = fns[j];

			// skip siblings
			if (collFog->areSiblings(fn->mID, other_fn->mID)) continue; 

			// skip if time interval don't overlap
			FoldEntity* other_cn = collFog->getFoldEntity(other_fn->mID);
			TimeInterval other_tInterval = chains[other_cn->entityIdx]->mFoldDuration;
			if (!overlap(tInterval, other_tInterval)) continue;

			// collision test using fold region
			Geom::Rectangle2 other_fArea = other_fn->properties["fArea"].value<Geom::Rectangle2>();
			if (Geom::IntrRect2Rect2::test(fArea, other_fArea))
			{
				collFog->addCollisionLink(fn, other_fn);
			}
		}
	}
}

QVector<FoldOption*> HBlock::generateFoldOptions()
{
	return QVector<FoldOption*>() << new FoldOption(mID + "_dummy");
}

void HBlock::applyFoldOption( FoldOption* fn )
{
	foldabilize();
}

double HBlock::getTimeLength()
{
	return nbMasters(this) - 1;
}

FdGraph* HBlock::getKeyframeScaffold( double t )
{
	// scaffolds from folded chains
	QVector<FdGraph*> foldedChains;
	for (int i = 0; i < chains.size(); i++)
	{
		double localT = getLocalTime(t, chains[i]->mFoldDuration);
		foldedChains << chains[i]->getKeyframeScaffold(t);
	}

	// combine 
	FdGraph* keyframeScaffold = combineDecomposition(foldedChains, baseMasterId, masterChainsMap);

	// delete folded chains
	foreach (FdGraph* c, foldedChains) delete c;

	return keyframeScaffold;
}

PatchNode* HBlock::getBaseMaster()
{
	return (PatchNode*)getNode(baseMasterId);
}
