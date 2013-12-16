#pragma once

#include "Graph.h"
#include "FdNode.h"
#include "FdLink.h"
#include "AABB.h"

class FdGraph : public Structure::Graph
{
public:
    FdGraph();
	FdGraph(FdGraph& other);
	virtual Graph* clone();

public:
	// accessors
	virtual void addLink(Structure::Node* n1, Structure::Node* n2);
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

