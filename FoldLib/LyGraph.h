#pragma once
#include "FdGraph.h"
#include "Numeric.h"

class LyGraph
{
public:
    LyGraph(FdGraph* scaffold, StrArray2D panelGroups, Vector3 up);

public:

	FdGraph* layerGraph;
	Vector3 upV;

	QVector<FdNode*> controlPanels;

public:
	// layers
	void createLayers();
};

