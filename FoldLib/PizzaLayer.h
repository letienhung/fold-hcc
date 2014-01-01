#pragma once
#include "LayerGraph.h"

class PizzaLayer : public LayerGraph
{
public:
    PizzaLayer(QVector<FdNode*> nodes, PatchNode* panel, QString id);
	~PizzaLayer();

	void buildDependGraph();

	QVector<Structure::Node*> getKeyFrameNodes(double t);

public: 
	PatchNode* mPanel; 
};
