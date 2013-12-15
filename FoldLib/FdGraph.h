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
	// accessors
	QVector<FdNode*> getFdNodes();

	// I/O
	void saveToFile(QString fname);
	void loadFromFile(QString fname);

	// aabb
	Geom::AABB computeAABB();

	// visualization
	void draw();

public:
	QString path;
	bool showAABB;
};

