#include "DcGraph.h"
#include "PatchNode.h"
#include "PizzaLayer.h"
#include "SandwichLayer.h"


DcGraph::DcGraph( FdGraph* scaffold, StrArray2D panelGroups, Vector3 up, QString id)
	: FdGraph(*scaffold)
{
	upV = up;
	mID = id;

	// merge control panels
	foreach( QVector<QString> panelGroup, panelGroups )
	{
		FdNode* mf = merge(panelGroup);
		mf->isCtrlPanel = true;

		if (mf->mType != FdNode::PATCH)	changeNodeType(mf);
		controlPanels.push_back((PatchNode*)mf);
	}

	// create layers
	createLayers();
	selId = -1;
}

DcGraph::~DcGraph()
{
	foreach (LayerGraph* l, layers)
		delete l;
}


void DcGraph::createLayers()
{
	// cut parts by control panels
 	foreach (PatchNode* panel, controlPanels)
	{
		Geom::Plane cutPlane = panel->mPatch.getPlane();

		foreach(FdNode* n, getFdNodes())
		{
			if (n->isCtrlPanel) continue;

			split(n, cutPlane, 0.1);
		}
	}
	
	// cut positions along pushing skeleton
	// append 1.0 to help group parts
	Geom::Box box = computeAABB().box();
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
	foreach (FdNode* n, getFdNodes())
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


LayerGraph* DcGraph::getSelLayer()
{
	if (selId >= 0 && selId < layers.size())
		return layers[selId];
	else
		return NULL;
}


FdGraph* DcGraph::activeScaffold()
{
	LayerGraph* selLayer = getSelLayer();
	if (selLayer) return selLayer->activeScaffold();
	else		  return this;
}

QStringList DcGraph::getLayerLabels()
{
	QStringList labels;
	foreach(LayerGraph* l, layers)
		labels.append(l->mID);

	return labels;
}

void DcGraph::selectLayer( QString id )
{
	// select layer named id
	selId = -1;
	for (int i = 0; i < layers.size(); i++)
	{
		if (layers[i]->mID == id)
		{
			selId = i;
			break;
		}
	}

	// disable selection on chains
	if (getSelLayer())
	{
		getSelLayer()->selectChain("");
	}
}
