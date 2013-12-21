#include "FoldManager.h"
#include "FdUtility.h"

FoldManager::FoldManager()
{
	scaffold = NULL;

	selectedId = 0;
	pushAxis = 0;
}


FoldManager::~FoldManager()
{
	clear();
}



void FoldManager::clear()
{
	foreach(LyGraph* lm, layerGraphs)
		delete lm;

	layerGraphs.clear();
}


void FoldManager::setScaffold( FdGraph* fdg )
{
	scaffold = fdg;
}

void FoldManager::fold()
{
	if (!scaffold) return;

	clear();

	if (pushAxis < 3)
	{
		Vector3 direct(0, 0, 0);
		direct[pushAxis] = 1;;
		createLayerGraphs(direct);
	}
	else
	{
		createLayerGraphs(Vector3(1,0,0));
		createLayerGraphs(Vector3(0,1,0));
		createLayerGraphs(Vector3(0,0,1));
	}
}


void FoldManager::setPushAxis( int d )
{
	pushAxis = d;
}

FdGraph* FoldManager::activeScaffold()
{
	if (selectedId >= 0 && selectedId < layerGraphs.size())
		return layerGraphs[selectedId]->layerGraph;
	else
		return NULL;
}


void FoldManager::createLayerGraphs(Vector3 pushDirect)
{
	Geom::AABB aabb = scaffold->computeAABB();
	Geom::Box box = aabb.box();
	int pushAId = aabb.box().getAxisId(pushDirect);
	
	// threshold
	double perpThr = 0.1;
	double layerHeightThr = 0.05;
	double clusterDistThr = aabb.radius() * 0.2;
	double areaThr = box.getPatch(pushAId, 0).area() * 0.2;

	// ==STEP 1==: nodes perp to pushing direction
	QVector<FdNode*> perpNodes;
	foreach (FdNode* n, scaffold->getFdNodes())
	{
		if (n->isPerpTo(pushDirect, 0.1))	perpNodes.push_back(n);
	}

	// perp positions
	Geom::Box shapeBox = scaffold->computeAABB().box();
	Geom::Segment skeleton = shapeBox.getSkeleton(pushAId);
	QMultiMap<double, FdNode*> posNodeMap;
	foreach (FdNode* n, perpNodes)
	{
		posNodeMap.insert(skeleton.getProjectedCoordinate(n->mBox.Center), n);
	}

	// ==STEP 2==: group perp nodes
	FdNodeArray2D perpGroups;
	QVector<FdNode*> perpGroup;
	double prePos = 0;
	foreach (double pos, posNodeMap.uniqueKeys())
	{
		// create a new group
		if (fabs(pos - prePos) > layerHeightThr && !perpGroup.isEmpty())
		{
			perpGroups.push_back(perpGroup);
			perpGroup.clear();
		}

		// add to current group
		foreach(FdNode* n, posNodeMap.values(pos))	perpGroup.push_back(n);

		prePos = pos;
	}
	// last group
	if (!perpGroup.isEmpty()) perpGroups.push_back(perpGroup);

	// ==STEP 3==: control panel group
	FdNodeArray2D panelGroups;
	foreach (QVector<FdNode*> pgroup, perpGroups)
	{
		foreach(QVector<FdNode*> cluster, clusterNodes(pgroup, clusterDistThr))
		{
			// accept if size is nontrivial
			Geom::AABB aabb;
			foreach(FdNode* n, cluster)
				aabb.add(n->computeAABB());
			Geom::Box box = aabb.box();
			int aid = box.getAxisId(pushDirect);
			double area = box.getPatch(aid, 0).area();

			if (area > areaThr) panelGroups.push_back(cluster);
		}
	}

	// reject rod structure
	FdNodeArray2D panelGroups2;
	foreach (QVector<FdNode*> panelGroup, panelGroups)
	{
		bool rodStruct = true;
		foreach(FdNode* n, panelGroup)	{
			if (n->mType == FdNode::PATCH){
				rodStruct = false;
				break;
			}
		}

		if (!rodStruct) panelGroups2.push_back(panelGroup);
	}
	
	// ==STEP 4==: create layer models
	// use all control panels
	layerGraphs.push_back(new LyGraph(scaffold, panelGroups, pushDirect));
	// exclude rod structures
	layerGraphs.push_back(new LyGraph(scaffold, panelGroups2, pushDirect));
}
