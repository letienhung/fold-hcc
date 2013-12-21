#include "LyGraph.h"
#include "PatchNode.h"


LyGraph::LyGraph( FdGraph* scaffold, StrArray2D panelGroups, Vector3 up)
{
	upV = up;

	// make copy of scaffold
	layerGraph = (FdGraph*)(scaffold->clone());
	
	// merge control panels
	foreach( QVector<QString> panelGroup, panelGroups )
	{
		FdNode* mf = layerGraph->merge(panelGroup);
		mf->isCtrlPanel = true;
		controlPanels.push_back(mf);
	}

	// create layers
	createLayers();
}


void LyGraph::createLayers()
{
	// cut parts by control panels
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

	// group parts into layers
	Geom::Box box = layerGraph->computeAABB().box();
	int upAId = box.getAxisId(upV);
	Geom::Segment upSklt = box.getSkeleton(upAId);

	QVector<double> cutPositions;
	foreach (FdNode* cp, controlPanels)
	{
		cutPositions.push_back(upSklt.getProjCoordinates(cp->center()));
	}

	FdNodeArray2D layerGroups;
	foreach (FdNode* n, layerGraph->getFdNodes())
	{
		if (n->isCtrlPanel) continue;

		double pos = upSklt.getProjCoordinates(n->center());

	}

	// create layers

}
