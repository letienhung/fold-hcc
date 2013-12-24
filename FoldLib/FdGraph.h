#pragma once

#include "Graph.h"
#include "FdNode.h"
#include "FdLink.h"
#include "AABB.h"
#include <QSharedPointer>

class FdGraph : public Structure::Graph
{
public:
	FdGraph(QString id = "");
	FdGraph(FdGraph& other);

	virtual Graph* clone();
	virtual void addLink(Structure::Node* n1, Structure::Node* n2);

public:
	// accessors
	QVector<FdNode*> getFdNodes();
	FdNode* addNode(SurfaceMeshModel* mesh, int method);

	// modifier
	FdNode*			 merge(QVector<QString> nids);
	QVector<FdNode*> split( FdNode* fn, Geom::Plane& plane, double thr );
	void			 changeNodeType(FdNode* n);

	// I/O
	void saveToFile(QString fname);
	void loadFromFile(QString fname);

	// aabb
	Geom::AABB computeAABB();

	// visualization
	void draw();
	void showCuboids(bool show);
	void showMeshes(bool show);
	void showScaffold(bool show);
	
public:
	QString path;
	bool showAABB;
};

typedef QSharedPointer<FdGraph> FdGraphPtr;