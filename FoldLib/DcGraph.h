#pragma once

#include "FdGraph.h"
#include "Numeric.h"
#include "LayerGraph.h"

class DcGraph : public FdGraph
{
public:
    DcGraph(FdGraph* scaffold, StrArray2D panelGroups, Vector3 up, QString id);

public:
	Vector3 upV;

	QVector<FdNode*> controlPanels;

	int selLayerId;
	QVector<LayerGraph*> layers;

private:
	// layers
	void createLayers();

public:
	// 
	FdGraph* activeScaffold();
	LayerGraph* getSelectedLayer();

	QStringList getLayerLabels();
	void selectLayer(QString id);
};

