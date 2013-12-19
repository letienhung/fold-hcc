#include "LayerModel.h"
#include "PatchNode.h"

LayerModel::LayerModel( FdGraph* scaffold, Vector3 direct )
{
	// make copy of scaffold
	layerGraph = (FdGraph*)(scaffold->clone());

	// push direction
	pushDirect = direct;
	Geom::AABB aabb = layerGraph->computeAABB();
	Geom::Box box = aabb.box();
	pushAId = box.getAxisId(pushDirect);

	// threshold
	perpThr = 0.1;
	layerHeightThr = 0.05;
	clusterDistThr = aabb.radius() * 0.2;
	areaThr = box.getPatch(pushAId, 0).area() * 0.2;

	// create layered model
	buildUp();
}

void LayerModel::buildUp()
{
	findControlNodes();
	groupControlNodes();
	createControlPanels();

	splitByControlPanels();
}

void LayerModel::findControlNodes()
{
	foreach (FdNode* n, layerGraph->getFdNodes())
	{
		if (n->isPerpTo(pushDirect, 0.1))
		{
			controlNodes.push_back(n);
			n->flipSelect();
		}
	}
}

void LayerModel::groupControlNodes()
{
	Geom::Box shapeBox = layerGraph->computeAABB().box();
	Geom::Segment skeleton = shapeBox.getSkeleton(pushAId);

	QMultiMap<double, FdNode*> posNodeMap;
	foreach (FdNode* n, controlNodes)
	{
		double pos = skeleton.getProjectedCoordinate(n->mBox.Center);
		posNodeMap.insert(pos, n);
	}

	controlGroups.clear();
	QVector<FdNode*> ctrlGroup;
	double prePos = 0;
	foreach (double pos, posNodeMap.uniqueKeys())
	{
		// create a new group
		if (fabs(pos - prePos) > layerHeightThr && !ctrlGroup.isEmpty())
		{
			controlGroups.push_back(ctrlGroup);
			ctrlGroup.clear();
		}

		// add to current group
		foreach(FdNode* n, posNodeMap.values(pos))
		{
			ctrlGroup.push_back(n);
		}

		prePos = pos;
	}

	// last group
	if (!ctrlGroup.isEmpty()) controlGroups.push_back(ctrlGroup);
}

void LayerModel::createControlPanels()
{
	foreach (QVector<FdNode*> cgroup, controlGroups)
	{
		foreach(QVector<FdNode*> cluster, clusterNodes(cgroup))
		{
			// compute the size
			Geom::AABB aabb;
			foreach(FdNode* n, cluster)
				aabb.add(n->computeAABB());
			Geom::Box box = aabb.box();
			double area = box.getPatch(pushAId, 0).area();

			// accept if size is above threshold
			if (area > areaThr)
			{
				FdNode* ctrlPanel = layerGraph->merge(cluster);
				ctrlPanel->isCtrlPanel = true; 
				controlPanels.push_back(ctrlPanel);
			}
		}
	}
}

QVector< QVector<FdNode*> > LayerModel::clusterNodes( QVector<FdNode*> nodes )
{
	QVector< QVector<FdNode*> > graphs;
	if (nodes.isEmpty()) return graphs;

	// tags used for searching
	QVector<bool> visited(nodes.size(), false);
	int nbVisited = 0;

	// initial current graph
	QVector<FdNode*> curr_g;
	curr_g.push_back(nodes[0]);
	visited[0] = true;
	nbVisited++;

	// search for connected node to current graph
	while(nbVisited < nodes.size())
	{
		int preN = nbVisited;
		for (int i = 0; i < nodes.size(); i++)
		{
			// skip visited node
			if (visited[i]) continue;

			foreach(FdNode* cgn, curr_g)
			{
				if (FdGraph::getDistance(nodes[i], cgn) < clusterDistThr)
				{
					curr_g.push_back(nodes[i]);
					visited[i] = true;
					nbVisited++;
				}
			}
		}

		// cannot find more nodes
		if (preN == nbVisited)
		{
			// save current graph
			graphs.push_back(curr_g);

			// reset current graph with an unvisited node
			for (int i = 0; i < nodes.size(); i++)
			{
				if (!visited[i])
				{
					curr_g.clear();
					curr_g.push_back(nodes[i]);
					visited[i] = true;
					nbVisited++;
					break;
				}
			}
		}
	}

	// save current graph
	graphs.push_back(curr_g);

	return graphs;
}

void LayerModel::splitByControlPanels()
{
	foreach (FdNode* cn, controlPanels)
	{
		PatchNode* cutNode = (PatchNode*)cn;
		Geom::Plane cutPlane = cutNode->mPatch.getPlane();

		foreach(FdNode* n, layerGraph->getFdNodes())
		{
			if (n->isCtrlPanel) continue;

			layerGraph->split(n, cutPlane, 0.1);
		}
	}
}
