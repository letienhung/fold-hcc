#include "FoldManager.h"
#include "FdUtility.h"

#include <QFileDialog>
#include <QDir>

#include "HBlock.h"

FoldManager::FoldManager()
{
	scaffold = NULL;
	dcScaffold = NULL;

	selDcIdx = -1;
	nbKeyframes = 15;
}

FoldManager::~FoldManager()
{
	clearDcGraphs();

	if (dcScaffold) delete dcScaffold;
}

void FoldManager::clearDcGraphs()
{
	foreach(DcGraph* dc, dcGraphs)
		delete dc;

	dcGraphs.clear();

	updateDcList();
}

// input
void FoldManager::setScaffold( FdGraph* fdg )
{
	scaffold = fdg;

	clearDcGraphs();
	updateDcList();
}

// masters
void FoldManager::identifyMasters(QString method, QString direct)
{
	if (scaffold == NULL)
	{
		emit(message("There is no input: load scaffold first."));
		return;
	}

	if (method == "Parallel")
	{
		Vector3 sqzV(0, 0, 0);
		if (direct == "X") sqzV[0] = 1;
		if (direct == "Y") sqzV[1] = 1;
		if (direct == "Z") sqzV[2] = 1;

		identifyParallelMasters(sqzV);
	}

	// visualize virtual parts
	emit(sceneChanged());
}

void FoldManager::identifyParallelMasters( Vector3 sqzV )
{
	// squeezing direction
	Geom::AABB aabb = scaffold->computeAABB();
	Geom::Box box = aabb.box();
	int sqzAId = aabb.box().getAxisId(sqzV);

	// clone scaffold for composition
	if (dcScaffold) delete dcScaffold;
	dcScaffold = (FdGraph*)scaffold->clone();

	// threshold
	double perpThr = 0.1;
	double blockHeightThr = 0.15;
	double adjacentThr = aabb.radius() * 0.1;
	double areaThr = box.getPatch(sqzAId, 0).area() * 0.2;

	// ==STEP 1==: nodes perp to squeezing direction
	QVector<FdNode*> perpNodes;
	foreach (FdNode* n, dcScaffold->getFdNodes())
	{
		// perp node
		if (n->isPerpTo(sqzV, perpThr))	
		{
			perpNodes << n;
		}
		// virtual rod nodes from patch edges
		else if (n->mType == FdNode::PATCH)
		{
			PatchNode* pn = (PatchNode*)n;
			foreach(RodNode* rn, pn->getEdgeRodNodes())
			{
				// add virtual rod nodes 
				if (rn->isPerpTo(sqzV, perpThr)) 
				{
					perpNodes << rn;
					dcScaffold->Structure::Graph::addNode(rn);
					rn->addTag(IS_EDGE_ROD);
				}
				// delete if not need
				else
					delete rn;
			}
		}
	}

	// ==STEP 2==: group perp nodes
	// perp positions
	Geom::Box shapeBox = dcScaffold->computeAABB().box();
	Geom::Segment skeleton = shapeBox.getSkeleton(sqzAId);
	QMultiMap<double, FdNode*> posNodeMap;
	foreach (FdNode* n, perpNodes)
	{
		posNodeMap.insert(skeleton.getProjCoordinates(n->mBox.Center), n);
	}

	// group
	FdNodeArray2D perpGroups;
	QVector<FdNode*> perpGroup;
	double prePos = 0;
	foreach (double pos, posNodeMap.uniqueKeys())
	{
		// create a new group
		if (fabs(pos - prePos) > blockHeightThr && !perpGroup.isEmpty())
		{
			perpGroups << perpGroup;
			perpGroup.clear();
		}

		// add to current group
		foreach(FdNode* n, posNodeMap.values(pos))	perpGroup.push_back(n);

		prePos = pos;
	}
	// last group
	if (!perpGroup.isEmpty()) perpGroups.push_back(perpGroup);

	// ==STEP 3==: master groups
	FdNodeArray2D masterGroups;
	foreach (QVector<FdNode*> pgroup, perpGroups)
	{
		// cluster based on adjacency
		foreach(QVector<FdNode*> cgroup, getConnectedGroups(pgroup, adjacentThr))
		{
			bool rejected = false;
			
			// reject if contains only edge rods
			//bool allEdgeRods = true;
			//foreach(FdNode* n, cgroup)
			//{
			//	if (!n->hasTag(IS_EDGE_ROD))
			//	{
			//		allEdgeRods = false;
			//		break;
			//	}
			//}
			//if (allEdgeRods) rejected = true;

			// reject if has trivial size
			if (!rejected)
			{
				Geom::AABB aabb;
				foreach(FdNode* n, cgroup)
					aabb.add(n->computeAABB());
				Geom::Box box = aabb.box();
				int aid = box.getAxisId(sqzV);
				double area = box.getPatch(aid, 0).area();
				if (area < areaThr) rejected = true;
			}

			// rejected
			if (rejected)
			{// remove virtual edge rod nodes
				foreach(FdNode* n, cgroup)
					if (n->hasTag(IS_EDGE_ROD))
						dcScaffold->removeNode(n->mID);
			}
			else // accept
				masterGroups << cgroup;
		}
	}

	// store ids of master groups
	masterIdGroups = getIds(masterGroups);
}

// decomposition
void FoldManager::decompose()
{
	QString id = "Dc_" + QString::number(dcGraphs.size());
	dcGraphs.push_back(new DcGraph(dcScaffold, masterIdGroups, id));

	// update ui
	updateDcList();
}

void FoldManager::foldbzSelBlock()
{
	BlockGraph* block = getSelBlock();
	if (block->mType == BlockGraph::H_BLOCK) 
	{
		HBlock* hblock = (HBlock*)block;
		hblock->foldabilize();
	}
}


void FoldManager::snapshotSelBlock( double t )
{
	BlockGraph* lg = getSelBlock();
	//if (lg) lg->snapshot(t);

	emit(sceneChanged());
}

void FoldManager::selectDcGraph( QString id )
{
	// selected lyGraph index
	selDcIdx = -1;
	for (int i = 0; i < dcGraphs.size(); i++)
	{
		if (dcGraphs[i]->mID == id)
		{	
			selDcIdx = i;
			break;
		}
	}

	// disable selection on block and chains
	selectBlock("");

	// update list
	updateBlockList();
	updateKeyframeList();

	// update scene
	emit(sceneChanged());
}

void FoldManager::selectBlock( QString id )
{
	if (getSelDcGraph()) 
	{
		getSelDcGraph()->selectBlock(id);
	}

	updateChainList();

	emit(sceneChanged());
}

void FoldManager::selectChain( QString id )
{ 
	if (getSelBlock())
	{
		getSelBlock()->selectChain(id);
	}

	emit(sceneChanged());
}

QStringList FoldManager::getDcGraphLabels()
{
	QStringList labels;
	foreach (DcGraph* dc, dcGraphs)	
		labels << dc->mID;

	// append string to select none
	labels << "--none--";

	return labels;
}

BlockGraph* FoldManager::getSelBlock()
{
	if (getSelDcGraph())
	{
		return getSelDcGraph()->getSelBlock();
	}
	else
		return NULL;
}

void FoldManager::generateKeyframes(int N)
{
	// generate key frames
	nbKeyframes = N;
	double step = 1.0 / nbKeyframes;

	// selected dc graph
	DcGraph* selDc = getSelDcGraph();
	if (!selDc) return;

	// forward message
	selDc->generateKeyframes(nbKeyframes);

	// emit signals
	updateKeyframeList();
}

void FoldManager::selectKeyframe( int idx )
{
	DcGraph* selDc = getSelDcGraph();
	if (!selDc) return;

	selDc->keyfameIdx = idx;

	emit(sceneChanged());
}

FdGraph* FoldManager::getSelKeyframe()
{
	DcGraph* selDc = getSelDcGraph();
	if (!selDc) return NULL;

	return selDc->getSelKeyframe();
}

DcGraph* FoldManager::getSelDcGraph()
{
	if (selDcIdx >= 0 && selDcIdx < dcGraphs.size())
		return dcGraphs[selDcIdx];
	else
		return NULL;
}

FdGraph* FoldManager::activeScaffold()
{
	DcGraph* selLy = getSelDcGraph();
	if (selLy)	return selLy->activeScaffold();
	else		return scaffold;
}

void FoldManager::foldabilize(bool withinAABB)
{
	foreach (DcGraph* dcg, dcGraphs)
		dcg->foldabilize(withinAABB);
}

void FoldManager::updateDcList()
{
	emit(DcGraphsChanged(getDcGraphLabels()));

	updateBlockList();
	updateKeyframeList();
}

void FoldManager::updateBlockList()
{
	QStringList layerLabels;
	if (getSelDcGraph()) 
		layerLabels = getSelDcGraph()->getBlockLabels();

	emit(blocksChanged(layerLabels));

	updateChainList();
}

void FoldManager::updateChainList()
{
	QStringList chainLables;
	if (getSelBlock())
		chainLables = getSelBlock()->getChainLabels();

	emit(chainsChanged(chainLables));
}


void FoldManager::updateKeyframeList()
{
	// selected dc graph
	DcGraph* selDc = getSelDcGraph();
	if (!selDc) return;

	emit(keyframesChanged(selDc->keyframes.size()));
}

void FoldManager::exportResultMesh()
{
	//if (results.isEmpty()) return;

	//// Get results output folder
	//QString dataPath = QFileDialog::getExistingDirectory(0, tr("Export Results"), getcwd());
	//// Create folder
	//dataPath += "//" + activeScaffold()->mID;
 //   QDir d(""); d.mkpath(dataPath);

	//for(int i = 0; i < results.size(); i++){
	//	// Create folder
	//	QString currPath = dataPath + "//result"+ QString::number(i+1);
	//	d.mkpath(currPath);
	//	for(int j = 0; j < results[i].size(); j++){
	//		QString filename = currPath + "//" + QString::number(j) + ".obj";
	//		results[i][j]->exportMesh(filename);
	//	}
	//}
}

void FoldManager::exportDepFOG()
{
	DcGraph* selDc = getSelDcGraph();
	if (selDc)
	{
		selDc->exportDepFOG();
	}
}

void FoldManager::exportCollFOG()
{
	DcGraph* selDc = getSelDcGraph();
	if (selDc)
	{
		selDc->exportCollFOG();
	}
}
