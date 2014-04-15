#pragma once

#include "Graph.h"
#include "FdNode.h"
#include "FdLink.h"
#include "AABB.h"
#include <QSharedPointer>
#include "FdUtility.h"

// FdGraph represents all segments of the input shape

class FdGraph : public Structure::Graph
{
public:
	FdGraph(QString id = "");
	FdGraph(FdGraph& other);

	virtual Graph* clone();
	FdGraph * deepClone();
	FdLink* addLink(FdNode* n1, FdNode* n2);

public:
	// accessors
	QVector<FdNode*> getFdNodes();
	FdNode* addNode(MeshPtr mesh, BOX_FIT_METHOD method = FIT_PCA);
	FdNode* addNode(MeshPtr mesh, Geom::Box& box);

	// modifier
	FdNode*			 merge(QVector<QString> nids);
	QVector<FdNode*> split(QString nid, Geom::Plane& plane);
	QVector<FdNode*> split(QString nid, QVector<Geom::Plane>& planes);
	void			 changeNodeType(FdNode* n);
	void             normalize(double f);
	void             translate(Vector3 v);

	// I/O
	void saveToFile(QString fname);
	void loadFromFile(QString fname);
	void exportMesh(QString fname);

	// aabb
	Geom::AABB computeAABB();

	// visualization
	void draw();
	void showCuboids(bool show);
	void showMeshes(bool show);
	void showScaffold(bool show);

	// configuration
	void restoreConfiguration();

	// debug
	void addDebugSegment(Geom::Segment seg);
	void addDebugSegments(QVector<Geom::Segment>& segs);
	void drawDebug();

public:
	QString path;
	bool showAABB;
};

typedef QSharedPointer<FdGraph> FdGraphPtr;