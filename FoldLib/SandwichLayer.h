#pragma once
#include "LayerGraph.h"

class SandwichLayer : public LayerGraph
{
public:
    SandwichLayer(QVector<FdNode*> parts, PatchNode* panel1, PatchNode* panel2, QString id, Geom::Box &bBox);

	void buildDependGraph();
	QVector<Structure::Node*> getKeyFrameNodes( double t );

public:
	PatchNode *mPanel1, *mPanel2;
};

