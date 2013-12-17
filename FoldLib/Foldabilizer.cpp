#include "Foldabilizer.h"

Foldabilizer::Foldabilizer( FdGraphPtr graph )
{
	scaffold = graph;
	direct = Z;

	// threshold
	perpThreshold = 0.1;
	layerHeightThreshold = 0.2;
	Geom::AABB aabb = graph->computeAABB();
	connectThreshold = aabb.radius() * 0.2;
}


void Foldabilizer::setDirection( DIRECTION i )
{
	direct = i;
}

SurfaceMesh::Vector3 Foldabilizer::getDirection()
{
	Vector3 v(0,0,0);
	v[direct] = 1;
	return v;
}

void Foldabilizer::run()
{
	setDirection(Z);
	findControlNodes();
	groupControlNodes();
	createControlPanels();
}

void Foldabilizer::findControlNodes()
{
	foreach (FdNode* n, scaffold->getFdNodes())
	{
		if (n->isPerpTo(getDirection(), 0.1))
		{
			controlNodes.push_back(n);
			n->flipSelect();
		}
	}
}


void Foldabilizer::groupControlNodes()
{
	Geom::Box shapeBox = scaffold->computeAABB().box();
	Geom::Segment skeleton = shapeBox.getSkeleton(direct);

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
		if (fabs(pos - prePos) > layerHeightThreshold && !ctrlGroup.isEmpty())
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

void Foldabilizer::createControlPanels()
{
	foreach (QVector<FdNode*> cg, controlGroups)
	{
		QVector< QVector<FdNode*> > cgraphs = extractConnectedGraphs(cg);
		if (cgraphs.size() == 1)
			scaffold->mergeNodes(cg);
	}
}

QVector< QVector<FdNode*> > Foldabilizer::extractConnectedGraphs( QVector<FdNode*> nodes )
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
				if (FdGraph::getDistance(nodes[i], cgn) < connectThreshold)
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