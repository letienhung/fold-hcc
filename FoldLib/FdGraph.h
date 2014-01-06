#pragma once

#include "Graph.h"
#include "FdNode.h"
#include "FdLink.h"
#include "AABB.h"
#include <QSharedPointer>
#include "FdUtility.h"

class FdGraph : public Structure::Graph
{
public:
	FdGraph(QString id = "");
	FdGraph(FdGraph& other);

	virtual Graph* clone();
	FdLink* addLink(FdNode* n1, FdNode* n2);

public:
	// accessors
	QVector<FdNode*> getFdNodes();
	FdNode* addNode(MeshPtr mesh, BOX_FIT_METHOD method = FIT_PCA);
	FdNode* addNode(MeshPtr mesh, Geom::Box& box);

	// modifier
	FdNode*			 merge(QVector<QString> nids);
	QVector<FdNode*> split(FdNode* fn, Geom::Plane& plane);
	void			 changeNodeType(FdNode* n);
	void             normalize(double f);
	void             translate(Vector3 v);

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

	// configuration
	void restoreConfiguration();

public:
	QString path;
	bool showAABB;
};

typedef QSharedPointer<FdGraph> FdGraphPtr;