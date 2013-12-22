#pragma once
#include "Graph.h"

class DepGraph : public Structure::Graph
{
public:
    DepGraph();
	DepGraph(DepGraph& other);
	Graph* clone();

public:
	void addPartAndFolingNodes(QString pid, QStringList fids);
	void addHingeLink(QString nid1, QString nid2);
	void addCollisionLink(QString nid1, QString nid2);
};
