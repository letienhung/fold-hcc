#pragma once
#include "FdGraph.h"
#include "Numeric.h"
#include "FdLayer.h"

class LyGraph
{
public:
    LyGraph(FdGraph* scaffold, StrArray2D panelGroups, Vector3 up, QString id);

public:
	FdGraph* layerGraph;
	Vector3 upV;
	QString id;

	QVector<FdNode*> controlPanels;

	int selLayerId;
	QVector<FdLayer*> layers;

private:
	// layers
	void createLayers();

public:
	// 
	FdGraph* activeScaffold();
	FdLayer* getSelectedLayer();

	QStringList getLayerLabels();
	void selectLayer(QString id);
};

