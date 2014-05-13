#include "HBlock.h"
#include "HChain.h"
#include "PatchNode.h"
#include "FdUtility.h"
#include "IntrRectRect.h"
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
		hc->setFoldDuration(masterTimeStamps[mid1], masterTimeStamps[mid2]);
		chains << hc;

		// map from master id to chain idx set
		masterChainsMap[mid1] << i;
		masterChainsMap[mid2] << i;
	}

	// initial collision graph
	collFogOrig = NULL;
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
	// collision graph
	buildCollisionGraph();

	// find optimal solution
	findOptimalSolution();

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
		// debug
		properties.remove("debugSegments");

		foreach(FoldOption* fn, chain->generateFoldOptions())
		{
			Geom::Rectangle fArea = fn->properties["fArea"].value<Geom::Rectangle>();

			// reject if goes out of barrier box
			if (withinAABB && !barrierBox.containsAll(fArea.getConners()))
				continue;

			// reject if collide with other masters
			// whose time stamp is within the time interval of fn
			bool reject = false;
			foreach (QString mid, masterTimeStamps.keys())
			{
				double mstamp = masterTimeStamps[mid];
				if (!within(mstamp, chain->mFoldDuration)) continue;

				Geom::Rectangle m_rect = ((PatchNode*)getNode(mid))->mPatch;
				if (fAreasIntersect(fArea, m_rect))
				{
					reject = true;
					break;
				}
			}
			if (reject) continue;

			// accept
			collFog->addNode(fn);
			collFog->addFoldLink(cn, fn);

			addDebugSegments(fArea.getEdgeSegments());
		}
	}

	// collision links
	QVector<FoldOption*> fns = collFog->getAllFoldOptions();
	for (int i = 0; i < fns.size(); i++)
	{
		FoldOption* fn = fns[i];
		FoldEntity* cn = collFog->getFoldEntity(fn->mID);
		Geom::Rectangle fArea = fn->properties["fArea"].value<Geom::Rectangle>();
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
			Geom::Rectangle other_fArea = other_fn->properties["fArea"].value<Geom::Rectangle>();
			if (fAreasIntersect(fArea, other_fArea))
			{
				collFog->addCollisionLink(fn, other_fn);
			}
		}
	}

	// debug
	std::cout << "# fold options / collision graph size = " 
		<< collFog->getAllFoldOptions().size() << std::endl;
}

void HBlock::findOptimalSolution()
{
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
	QVector<QVector<int> > qs = cliquer.getMinWeightMaxCliques();

	// fold solution
	foldSolution.clear();
	foldSolution.resize(chains.size());
	if (collFogOrig) delete collFogOrig;
	collFogOrig = (FoldOptionGraph*)collFog->clone();
	foreach(int idx, qs[0])
	{
		FoldOption* fn = fns[idx];
		fn->addTag(SELECTED_FOLD_OPTION);
		FoldEntity* cn = collFog->getFoldEntity(fn->mID);
		foldSolution[cn->entityIdx] = fn;
	}
}

QVector<Geom::Box> HBlock::getFoldVolume()
{
	// get master pairs
	QVector<QVector<QString> > mid_pairs;
	QList<QString> mids = masterTimeStamps.keys();
	for (int i = 0; i < mids.size(); i++)
	{
		for (int j = i + 1; j < mids.size(); j++)
		{
			bool connected = false;
			foreach (ChainGraph* c, chains)
			{
				if (c->getNode(mids[i]) && c->getNode(mids[j]))
				{
					connected = true;
					break;
				}
			}

			if (connected)
				mid_pairs << (QVector<QString>() << mids[i] << mids[j]);
		}
	}

	// each pair of masters connected by chains form a folding box
	QVector<Geom::Box> fV;
	Geom::IntrRectRect RR;
	foreach (QVector<QString> mid_pair, mid_pairs)
	{
		QString mid1 = mid_pair.front();
		QString mid2 = mid_pair.last();

		Geom::Rectangle rect1 = ((PatchNode*)getNode(mid1))->mPatch;
		Geom::Rectangle rect2 = ((PatchNode*)getNode(mid2))->mPatch;

		// make sure two rectangles are coplanar
		Geom::Rectangle rect2on1 = rect1.get3DRectangle(rect1.get2DRectangle(rect2));

		// intersection
		QVector<Vector3> intrPnts = RR.test(rect1, rect2on1);
		if (intrPnts.size() != 4) continue;

		// base rectangle
		Geom::Rectangle base_rect(intrPnts);
		
		// upright and height
		Vector3 n = base_rect.Normal;
		Vector3 c2c = rect2.Center - rect1.Center;
		double height = dot(n, c2c);
		if (height < 0) { n *= -1; height *= -1;}

		// box
		fV << Geom::Box(base_rect, n, height);
	}

	return fV;
}

QVector<FoldOption*> HBlock::generateFoldOptions()
{
	// fold option
	FoldOption* fn = new FoldOption(mID + "_00");

	// fold volume
	fn->properties["fVolume"].setValue(getFoldVolume());

	return QVector<FoldOption*>() << fn;
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
		foldedChains << chains[i]->getKeyframeScaffold(localT);
	}

	// combine 
	FdGraph* keyframeScaffold = combineDecomposition(foldedChains, baseMasterId, masterChainsMap);

	// delete folded chains
	foreach (FdGraph* c, foldedChains) delete c;

	return keyframeScaffold;
}

bool HBlock::fAreasIntersect( Geom::Rectangle& rect1, Geom::Rectangle& rect2 )
{
	Geom::Rectangle base_rect = getBaseMaster()->mPatch;
	
	Geom::Rectangle2 r1 = base_rect.get2DRectangle(rect1);
	Geom::Rectangle2 r2 = base_rect.get2DRectangle(rect2);

	return Geom::IntrRect2Rect2::test(r1, r2);
}

void HBlock::exportCollFOG()
{
	QString filePath = path + "/" + mID + "_collision";
	collFogOrig->saveAsImage(filePath + "_Orig");
	collFog->saveAsImage(filePath);
}
