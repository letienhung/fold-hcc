#pragma once

#include "Graph.h"
#include "FdNode.h"
#include "FdLink.h"

class FdGraph : public Structure::Graph
{
public:
    FdGraph();

	QVector<FdNode*> getFdNodes();
};

