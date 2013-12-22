#include "LyGraph.h"
#include "PatchNode.h"
#include "PizzaLayer.h"
#include "SandwichLayer.h"


LyGraph::LyGraph( FdGraph* scaffold, StrArray2D panelGroups, Vector3 up, QString id)
{
	upV = up;
	this->id = id;

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
	selLayerId = -1;
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
	
	// cut positions along pushing skeleton
	// append 1.0 to help group parts
	Geom::Box box = layerGraph->computeAABB().box();
	int upAId = box.getAxisId(upV);
	Geom::Segment upSklt = box.getSkeleton(upAId);

	QVector<double> cutPos;
	foreach (FdNode* cp, controlPanels)
	{
		cutPos.push_back(upSklt.getProjCoordinates(cp->center()));
	}
	cutPos.push_back(1.0);

	// group parts into layers
	FdNodeArray2D layerGroups(cutPos.size());
	foreach (FdNode* n, layerGraph->getFdNodes())
	{
		if (n->isCtrlPanel) continue;

		double pos = upSklt.getProjCoordinates(n->center());
		for (int i = 0; i < cutPos.size(); i++)
		{
			if (pos < cutPos[i])
			{
				layerGroups[i].push_back(n);
				break;
			}
		}
	}

	// clear layers
	layers.clear();

	// first layer is pizza
	QVector<FdNode*> lgroup = layerGroups.front();
	if (!lgroup.isEmpty())
	{
		QString id = QString::number(layers.size()) + ":pizza";
		layers.push_back(new PizzaLayer(lgroup, controlPanels.front(), id));
	}

	// sandwiches
	for (int i = 1; i < layerGroups.size()-1; i++)
	{
		lgroup = layerGroups[i];
		if (!lgroup.isEmpty())
		{
			QString id = QString::number(layers.size()) + ":sandwich";
			layers.push_back(new SandwichLayer(lgroup, controlPanels[i-1], controlPanels[i], id));
		}
	}

	// first layer is pizza
	lgroup = layerGroups.last();
	if (!lgroup.isEmpty())
	{
		QString id = QString::number(layers.size()) + ":pizza";
		layers.push_back(new PizzaLayer(lgroup, controlPanels.last(), id));
	} 
}


FdLayer* LyGraph::getSelectedLayer()
{
	if (selLayerId >= 0 && selLayerId < layers.size())
		return layers[selLayerId];
	else
		return NULL;
}


FdGraph* LyGraph::activeScaffold()
{
	FdLayer* selLayer = getSelectedLayer();
	if (selLayer) return selLayer->layer;
	else		  return layerGraph;
}

QStringList LyGraph::getLayerLabels()
{
	QStringList labels;
	foreach(FdLayer* l, layers)
		labels.append(l->id);

	return labels;
}

void LyGraph::selectLayer( QString id )
{
	selLayerId = -1;
	for (int i = 0; i < layers.size(); i++)
	{
		if (layers[i]->id == id)
		{
			selLayerId = i;
			break;
		}
	}
}
