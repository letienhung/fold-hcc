#include "Foldabilizer.h"

Foldabilizer::Foldabilizer( FdGraphPtr graph )
{
	scaffold = graph;
	direct = Z;

	// threshold
	perpThreshold = 0.1;
	layerHeightThreshold = 0.2;
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
	foreach (double pos, posNodeMap.keys())
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
		scaffold->mergeNodes(cg);
	}
}