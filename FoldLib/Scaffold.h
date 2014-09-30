#pragma once

#include "Graph.h"
#include "ScaffoldNode.h"
#include "ScaffoldLink.h"
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
	ScaffoldLink* addLink(ScaffoldNode* n1, ScaffoldNode* n2);

public:
	// accessors
	QVector<ScaffoldNode*> getScfdNodes();
	ScaffoldNode* getFdNode(QString id);
	ScaffoldNode* addNode(MeshPtr mesh, BOX_FIT_METHOD method = FIT_PCA);
	ScaffoldNode* addNode(MeshPtr mesh, Geom::Box& box);

	// modifier
	void changeNodeType(ScaffoldNode* n);
	PatchNode* changeRodToPatch(RodNode* n, Vector3 v);
	void translate(Vector3 v, bool withMesh = true);
	void unwrapBundleNodes();
	ScaffoldNode* wrapAsBundleNode(QVector<QString> nids, Vector3 v = Vector3(0, 0, 0));
	QVector<ScaffoldNode*> split(QString nid, Geom::Plane& plane);
	QVector<ScaffoldNode*> split(QString nid, QVector<Geom::Plane>& planes);

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
#define DEBUG_SCAFFOLDS "debugScaffolds"