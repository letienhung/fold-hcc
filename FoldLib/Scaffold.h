#pragma once

#include "Graph.h"
#include "ScaffNode.h"
#include "ScaffLink.h"
#include "AABB.h"
#include <QSharedPointer>
#include "FdUtility.h"

// FdGraph represents all segments of the input shape

class Scaffold : public Structure::Graph
{
public:
	Scaffold(QString id = "");
	virtual ~Scaffold();
	Scaffold(Scaffold& other);

	virtual Graph* clone() override;
	ScaffLink* addLink(ScaffNode* n1, ScaffNode* n2);

public:
	// accessors
	QVector<ScaffNode*> getScaffNodes();
	ScaffNode* getFdNode(QString id);
	ScaffNode* addNode(MeshPtr mesh, BOX_FIT_METHOD method = FIT_PCA);
	ScaffNode* addNode(MeshPtr mesh, Geom::Box& box);

	// modifier
	void changeNodeType(ScaffNode* n);
	PatchNode* changeRodToPatch(RodNode* n, Vector3 v);
	void translate(Vector3 v, bool withMesh = true);
	void unwrapBundleNodes();
	ScaffNode* wrapAsBundleNode(QVector<QString> nids, Vector3 v = Vector3(0, 0, 0));
	QVector<ScaffNode*> split(QString nid, Geom::Plane& plane);
	QVector<ScaffNode*> split(QString nid, QVector<Geom::Plane>& planes);

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

typedef QSharedPointer<Scaffold> ScaffoldPtr;
Q_DECLARE_METATYPE(QVector<Scaffold*>)

#define DEBUG_POINTS "debugPoints"
#define DEBUG_SEGS "debugSegments"
#define DEBUG_RECTS "debugRectangles"
#define DEBUG_PLANES "debugPlanes"
#define DEBUG_BOXES "debugBoxes"
#define DEBUG_SCAFFS "debugScaffolds"