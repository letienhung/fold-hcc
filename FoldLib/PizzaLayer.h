#pragma once
#include "LayerGraph.h"

class PizzaLayer : public LayerGraph
{
public:
    PizzaLayer(QVector<FdNode*> parts, PatchNode* panel, QString id, Geom::Box &bBox);
	~PizzaLayer();

	void buildDependGraph();
	QVector<Structure::Node*> getKeyFrameNodes( double t );

public: 
	PatchNode* mPanel; 
};
