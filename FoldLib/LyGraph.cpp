#include "LyGraph.h"
#include "PatchNode.h"


LyGraph::LyGraph( FdGraph* scaffold, FdNodeArray2D panelGroups, Vector3 up)
{
	upV = up;

	// make copy of scaffold
	layerGraph = (FdGraph*)(scaffold->clone());
	
	// merge control panels
	foreach( QVector<FdNode*> panelGroup, panelGroups )
	{
		FdNode* mf = layerGraph->merge(panelGroup);
		controlPanels.push_back(mf);
	}
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
}
