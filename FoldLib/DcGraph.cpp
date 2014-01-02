#include "DcGraph.h"
#include "PatchNode.h"
#include "PizzaLayer.h"
#include "SandwichLayer.h"
#include <QFileInfo>


DcGraph::DcGraph( FdGraph* scaffold, StrArray2D panelGroups, Vector3 up, QString id)
	: FdGraph(*scaffold)
{
	path = QFileInfo(path).absolutePath();
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
	QString id_first = "Pz-" + QString::number(layers.size());
	PizzaLayer* pl_first = new PizzaLayer(layerGroups.front(), controlPanels.front(), id_first);
	pl_first->path = path;
	layers.push_back(pl_first);

	// sandwiches
	for (int i = 1; i < layerGroups.size()-1; i++)
	{
		QString id = "Sw-" + QString::number(layers.size());
		SandwichLayer* sl = new SandwichLayer(layerGroups[i], controlPanels[i-1], controlPanels[i], id);
		sl->path = path;
		layers.push_back(sl);
	}

	// first layer is pizza
	QString id_last = "Pz-" + QString::number(layers.size());
	PizzaLayer* pl_last = new PizzaLayer(layerGroups.last(), controlPanels.last(), id_last);
	pl_last->path = path;
	layers.push_back(pl_last);
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

FdGraph* DcGraph::getKeyFrame( double t )
{
	// evenly distribute the time among layers
	double step = 1.0 / layers.size();
	QVector<double> layerStarts;
	for (int i = 0; i < layers.size(); i++)
		layerStarts << i * step;
	layerStarts << 1.0;

	// get folded nodes of each layer
	// the folding base is the first control panel
	QVector< QVector<Structure::Node*> > knodes;
	for (int i = 0; i < layers.size(); i++)
	{
		double start = layerStarts[i];
		double end = layerStarts[i+1];

		if (t < start)
			knodes << layers[i]->getKeyFrameNodes(0);

		if (t >= start && t < end)
		{
			double t_layer = (t - start) / step;
			knodes << layers[i]->getKeyFrameNodes(t_layer);
		}

		if (t >= end)
			knodes << layers[i]->getKeyFrameNodes(1);
	}

	// compute offset of each control panel
	// and remove redundant control panels
	QVector<Vector3> panelDeltas;
	for (int i = 0; i < controlPanels.size(); i++)
	{
		QString panel_id = controlPanels[i]->mID;
		
		// copy1 in layer[i]
		FdNode* panelCopy1;
		int idx1;
		QVector<Structure::Node*> &lnodes1 = knodes[i];
		for(int j = 0; j < lnodes1.size(); j++)
		{
			if (lnodes1[j]->hasId(panel_id))
			{
				panelCopy1 = (FdNode*)lnodes1[j];
				idx1 = j;
			}
		}

		// copy2 in layer[i+1]
		FdNode* panelCopy2;
		int idx2;
		QVector<Structure::Node*> &lnodes2 = knodes[i+1];
		for(int j = 0; j < lnodes2.size(); j++)
		{
			if (lnodes2[j]->hasId(panel_id))
			{
				panelCopy2 = (FdNode*)lnodes2[j];
				idx2 = j;
			}
		}


		// delta
		panelDeltas << panelCopy1->center() - panelCopy2->center();

		// remove copy2
		lnodes2.remove(idx2);
	}

	// shift layers and add nodes into scaffold
	FdGraph *key_graph = new FdGraph();
	Vector3 offset(0, 0, 0);
	for (int i = 1; i < knodes.size(); i++)
	{
		offset += panelDeltas[i-1];

		QVector<Structure::Node*> &lnodes = knodes[i];
		for (int j = 0; j < lnodes.size(); j++)
		{
			FdNode* n = (FdNode*)lnodes[j];
			n->mBox.translate(offset);
			n->deformMesh();
			n->createScaffold();

			key_graph->Structure::Graph::addNode(n);
		}
	}

	return key_graph;
}

void DcGraph::fold()
{
	foreach(LayerGraph* layer, layers)
		layer->fold();
}