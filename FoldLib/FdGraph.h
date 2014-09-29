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
	virtual ~FdGraph();
	FdGraph(FdGraph& other);

	virtual Graph* clone();
	FdLink* addLink(FdNode* n1, FdNode* n2);

public:
	// accessors
	QVector<FdNode*> getFdNodes();
	FdNode* getFdNode(QString id);
	FdNode* addNode(MeshPtr mesh, BOX_FIT_METHOD method = FIT_PCA);
	FdNode* addNode(MeshPtr mesh, Geom::Box& box);

	// modifier
	void changeNodeType(FdNode* n);
	PatchNode* changeRodToPatch(RodNode* n, Vector3 v);
	void translate(Vector3 v, bool withMesh = true);
	void unwrapBundleNodes();
	FdNode* wrapAsBundleNode(QVector<QString> nids, Vector3 v = Vector3(0, 0, 0));
	QVector<FdNode*> split(QString nid, Geom::Plane& plane);
	QVector<FdNode*> split(QString nid, QVector<Geom::Plane>& planes);

	// I/O
	void saveToFile(QString fname);
	void loadFromFile(QString fname);
	void exportMesh(QString fname);

	// aabb
	Geom::AABB computeAABB();

	// configuration
	void restoreConfiguration();

	// visualization
	virtual void draw() override;
	void drawAABB();
	void showCuboids(bool show);
	void showMeshes(bool show);
	void showScaffold(bool show);

	// rendering
	void hideEdgeRods();

	// debug
	void drawDebug();

public:
	QString path;
	bool showAABB;
};

typedef QSharedPointer<FdGraph> FdGraphPtr;
Q_DECLARE_METATYPE(QVector<FdGraph*>)

#define DEBUG_POINTS "debugPoints"
#define DEBUG_SEGS "debugSegments"
#define DEBUG_RECTS "debugRectangles"
#define DEBUG_PLANES "debugPlanes"
#define DEBUG_BOXES "debugBoxes"
#define DEBUG_SCAFFOLDS "debugScaffolds"