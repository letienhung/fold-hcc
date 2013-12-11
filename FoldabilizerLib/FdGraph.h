#pragma once

#include "Graph.h"
#include "FdNode.h"
#include "FdLink.h"

class FdGraph : public Structure::Graph
{
public:
    FdGraph();

public:
	QVector<FdNode*> getFdNodes();
	void saveToFile(QString fname);

public:
	QString path;
};

