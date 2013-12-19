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
	FdNode* addNode(SurfaceMeshModel* mesh, int method);

	// modifier
	FdNode*			 merge(QVector<FdNode*> ns);
	QVector<FdNode*> split( FdNode* fn, Geom::Plane& plane, double thr );


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

	// helpers
	static Geom::Segment getDistSegment(FdNode* n1, FdNode* n2);
	static double getDistance(FdNode* n1, FdNode* n2);
	
public:
	QString path;
	bool showAABB;
};

typedef QSharedPointer<FdGraph> FdGraphPtr;