#include "DcGraph.h"
#include "PatchNode.h"
#include "PizzaLayer.h"
#include "SandwichLayer.h"
#include <QFileInfo>
#include "FdUtility.h"


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
	splitPartsByPanels();
	createLayers();

	selId = -1;
}

DcGraph::~DcGraph()
{
	foreach (LayerGraph* l, layers)
		delete l;
}

void DcGraph::splitPartsByPanels()
{
	// cut parts by control panels: use two surface planes
	// and collect chopped pieces which should be merged into control panels
	QVector<PatchNode*> newPanels;
	foreach (PatchNode* panel, controlPanels)
	{
		double thr = panel->mBox.getExtent(panel->mPatch.Normal);
		Geom::Plane patchPlane = panel->mPatch.getPlane();
		Geom::Plane cutPlane1 = panel->getSurfacePlane(true);
		Geom::Plane cutPlane2 = panel->getSurfacePlane(false);

		// cut plane 1
		QVector<QString> mergeParts;
		foreach(FdNode* n, getFdNodes())
		{
			if (n->isCtrlPanel) continue;
			QVector<FdNode*> splitted = split(n, cutPlane1);

			foreach (FdNode* spn, splitted)
			{
				double dist = patchPlane.distanceTo(spn->mBox.Center);
				if (dist < thr)
				{
					mergeParts << spn->mID;
				}
			}
		}
		if (!mergeParts.isEmpty())
		{
			mergeParts.push_front(panel->mID);
			PatchNode* newPanel = (PatchNode*)merge(mergeParts);
			Structure::Graph::replaceNode(panel, newPanel);
			panel = newPanel;
			panel->isCtrlPanel = true;
		}

		// cut plane2
		mergeParts.clear();
		foreach(FdNode* n, getFdNodes())
		{
			if (n->isCtrlPanel) continue;
			QVector<FdNode*> splitted = split(n, cutPlane2);

			foreach (FdNode* spn, splitted)
			{
				if (patchPlane.distanceTo(spn->mBox.Center) < thr)
				{
					mergeParts << spn->mID;
				}
			}
		}
		if (!mergeParts.isEmpty())
		{
			mergeParts.push_front(panel->mID);
			PatchNode* newPanel = (PatchNode*)merge(mergeParts);
			Structure::Graph::replaceNode(panel, newPanel);
			panel = newPanel;
			panel->isCtrlPanel = true;
		}

		// save the new panel
		newPanels << panel;
	}

	// update control panles
	controlPanels = newPanels;
}


QVector<FdNode*> DcGraph::mergeCoplanarParts( QVector<FdNode*> ns, PatchNode* panel )
{
	// classify
	QVector<RodNode*>	rootRod;
	QVector<PatchNode*> rootPatch;
	double thr = panel->mBox.getExtent(panel->mPatch.Normal) * 2;
	foreach (FdNode* n, ns)
	{
		if (getDistance(n, panel) < thr)
		{
			if (n->mType == FdNode::PATCH) 
				rootPatch << (PatchNode*)n;
			else rootRod << (RodNode*)n;
		}
	}

	// RANSAC like strategy: use a few samples to produce fitting model, which is a plane
	// a plane is fitted by either one root patch or two coplanar root rods
	QVector<Geom::Plane> fitPlanes;
	foreach(PatchNode* pn, rootPatch) fitPlanes << pn->mPatch.getPlane();
	for (int i = 0; i < rootRod.size(); i++){
		for (int j = i+1; j< rootRod.size(); j++)
		{
			Vector3 v1 = rootRod[i]->mRod.Direction;
			Vector3 v2 = rootRod[j]->mRod.Direction;
			double dotProd = fabs(dot(v1, v2));
			if (dotProd > 0.9)
			{
				Vector3 v = (v1 + v2) * 0.5;
				Vector3 u = rootRod[i]->mRod.Center - rootRod[j]->mRod.Center;
				Vector3 normal = cross(u, v).normalized();
				fitPlanes << Geom::Plane(rootRod[i]->mRod.Center, normal);
			}
		}
	}

	// search for coplanar parts for each fit plane
	// add group them into connected components
	QVector< QSet<QString> > copGroups;
	double distThr = computeAABB().radius() * 0.1;
	foreach(Geom::Plane plane, fitPlanes)
	{
		QVector<FdNode*> copNodes;
		foreach(FdNode* n, ns)
			if (onPlane(n, plane)) copNodes << n;

		foreach(QVector<FdNode*> group, clusterNodes(copNodes, distThr))
		{
			QSet<QString> groupIds;
			foreach(FdNode* n, group) groupIds << n->mID;
			copGroups << groupIds;
		}
	}

	// set cover: prefer large subsets
	QVector<bool> deleted(copGroups.size(), false);
	int count = copGroups.size();
	QVector<int> subsetIndices;
	while(count > 0)
	{
		// search for the largest group
		int maxSize = -1;
		int maxIdx = -1;
		for (int i = 0; i < copGroups.size(); i++)
		{
			if (deleted[i]) continue;
			if (copGroups[i].size() > maxSize)
			{
				maxSize = copGroups[i].size();
				maxIdx = i;
			}
		}

		// save selected subset
		subsetIndices << maxIdx;
		deleted[maxIdx] = true;
		count--;

		// delete overlapping subsets
		for (int i = 0; i < copGroups.size(); i++)
		{
			if (deleted[i]) continue;

			QSet<QString> maxGroup = copGroups[maxIdx];
			maxGroup.intersect(copGroups[i]);
			if (!maxGroup.isEmpty())
			{
				deleted[i] = true;
				count--;
			}
		}
	}

	// merge each selected group
	QVector<FdNode*> mnodes;
	foreach(int i, subsetIndices)
		mnodes << merge(copGroups[i].toList().toVector());

	return mnodes;
}


void DcGraph::createLayers()
{
	// cut positions along pushing skeleton
	// append 1.0 to help group parts
	Geom::Box box = computeAABB().box();
	int upAId = box.getAxisId(upV);
	Geom::Segment upSklt = box.getSkeleton(upAId);
	QVector<double> cutPos;
	foreach (FdNode* cp, controlPanels)
		cutPos.push_back(upSklt.getProjCoordinates(cp->center()));
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

	// merge coplanar parts within each layer 
	FdNodeArray2D mergedGroups;
	mergedGroups << mergeCoplanarParts(layerGroups[0], controlPanels[0]);
	for (int i = 1; i < layerGroups.size(); i++)
		mergedGroups << mergeCoplanarParts(layerGroups[i], controlPanels[i-1]);
	layerGroups = mergedGroups;

	// clear layers
	layers.clear();

	// barrier box
	Geom::Box bBox = box.scaled(1.01);

	// first layer is pizza
	QString id_first = "Pz-" + QString::number(layers.size());
	PizzaLayer* pl_first = new PizzaLayer(layerGroups.front(), controlPanels.front(), id_first, bBox);
	pl_first->path = path;
	layers.push_back(pl_first); 

	// sandwiches
	for (int i = 1; i < layerGroups.size()-1; i++)
	{
		QString id = "Sw-" + QString::number(layers.size());
		SandwichLayer* sl = new SandwichLayer(layerGroups[i], controlPanels[i-1], controlPanels[i], id, bBox);
		sl->path = path;
		layers.push_back(sl);
	}

	// last layer is pizza
	QString id_last = "Pz-" + QString::number(layers.size());
	PizzaLayer* pl_last = new PizzaLayer(layerGroups.last(), controlPanels.last(), id_last, bBox);
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
	// if the end layer is empty, assign zero time interval to it
	bool empty_front = (layers.front()->nbNodes() == 1);
	bool empty_back = (layers.back()->nbNodes() == 1);
	QVector<double> layerStarts;
	if (empty_front && empty_back)
	{
		layerStarts = getEvenDivision(layers.size() - 2);
		layerStarts.insert(layerStarts.begin(), 0);
		layerStarts.append(1);
	}
	else if (empty_front)
	{
		layerStarts = getEvenDivision(layers.size() - 1);
		layerStarts.insert(layerStarts.begin(), 0);
	}
	else if (empty_back)
	{
		layerStarts = getEvenDivision(layers.size() - 1);
		layerStarts.append(1);
	}
	else
	{
		layerStarts = getEvenDivision(layers.size());
	}

	// get folded nodes of each layer
	// the folding base is the first control panel
	QVector< QVector<Structure::Node*> > knodes;
	for (int i = 0; i < layers.size(); i++)
	{
		double lt = getLocalTime(t, layerStarts[i], layerStarts[i+1]);
		knodes << layers[i]->getKeyFrameNodes(lt);
	}

	// compute offset of each control panel
	// and remove redundant control panels
	QVector<Vector3> panelDeltas;
	for (int i = 0; i < controlPanels.size(); i++)
	{
		QString panel_id = controlPanels[i]->mID;
		
		// copy1 in layer[i]
		FdNode* panelCopy1 = NULL;
		int idx1 = -1;
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
		FdNode* panelCopy2 = NULL;
		int idx2 = -1;
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
		Vector3 delta(0, 0, 0);
		if (panelCopy1 && panelCopy2) 
			delta = panelCopy1->center() - panelCopy2->center();
		panelDeltas << delta;

		// remove copy2
		if (idx2 >= 0 && idx2 < lnodes2.size())
			lnodes2.remove(idx2);
	}

	// shift layers and add nodes into scaffold
	FdGraph *key_graph = new FdGraph();
	Vector3 offset(0, 0, 0);
	for (int i = 0; i < knodes.size(); i++)
	{
		// keep the first layer but shift others
		if (i > 0) offset += panelDeltas[i-1];

		QVector<Structure::Node*> &lnodes = knodes[i];
		for (int j = 0; j < lnodes.size(); j++)
		{
			FdNode* n = (FdNode*)lnodes[j];
			n->mBox.translate(offset);
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