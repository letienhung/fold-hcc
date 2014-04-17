#pragma once

#include "FdGraph.h"
#include "PatchNode.h"
#include "Numeric.h"
#include "LayerGraph.h"

// DcGraph is the Decomposition graph of a Concertina
// including base patches and layers

class DcGraph : public FdGraph
{
public:
    DcGraph(FdGraph* scaffold, StrArray2D panelGroups, Vector3 pushV, QString id);
	~DcGraph();

public:
	int pushAId;
	QVector<PatchNode*> controlPanels;

	int selId;
	QVector<LayerGraph*> layers;

public:
	// layers
	void createLayers();
	QVector<FdNode*> mergeCoplanarParts(QVector<FdNode*> ns, PatchNode* panel);

	// fold
	void foldabilize();

	// key frame
	FdGraph* getKeyFrame(double t);

public:
	FdGraph* activeScaffold();
	LayerGraph* getSelLayer();

	QStringList getLayerLabels();
	void selectLayer(QString id);
};

