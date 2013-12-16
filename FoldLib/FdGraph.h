#pragma once

#include "Graph.h"
#include "FdNode.h"
#include "FdLink.h"
#include "AABB.h"
#include <QSharedPointer>

class FdGraph : public Structure::Graph
{
public:
    FdGraph();
	FdGraph(FdGraph& other);

	virtual Graph* clone();
	virtual void addLink(Structure::Node* n1, Structure::Node* n2);

public:
	// accessors
	QVector<FdNode*> getFdNodes();

	// modifier
	void mergeNodes(QVector<FdNode*> ns);
	void addNode(SurfaceMeshModel* mesh, int method);

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

typedef QSharedPointer<FdGraph> FdGraphPtr;