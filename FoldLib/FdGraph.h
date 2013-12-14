#pragma once

#include "Graph.h"
#include "FdNode.h"
#include "FdLink.h"
#include "AABB.h"

class FdGraph : public Structure::Graph
{
public:
    FdGraph();

public:
	QVector<FdNode*> getFdNodes();
	void saveToFile(QString fname);
	void loadFromFile(QString fname);

	Geom::AABB computeAABB();

public:
	QString path;
};

