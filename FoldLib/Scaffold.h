#pragma once

#include "Graph.h"
#include "ScaffNode.h"
#include "ScaffLink.h"
#include "AABB.h"
#include <QSharedPointer>
#include "FdUtility.h"
#include "VisualDebugger.h"

// FdGraph represents all segments of the input shape

class Scaffold : public Structure::Graph
{
public:
	Scaffold(QString id = "");
	Scaffold(Scaffold& other);
	Scaffold(QVector<Scaffold*> scaffs, QString baseMid, QMap<QString, QSet<int> >& masterScaffMap); 
	virtual ~Scaffold();
		
	virtual Graph* clone() override;
	ScaffLink* addLink(ScaffNode* n1, ScaffNode* n2);

public:
	// accessors
	QVector<ScaffNode*> getScaffNodes();
	ScaffNode* getScaffNode(QString id);
	ScaffNode* addNode(MeshPtr mesh, BOX_FIT_METHOD method = FIT_PCA);
	ScaffNode* addNode(MeshPtr mesh, Geom::Box& box);

	// modifier
	void changeNodeType(ScaffNode* n);
	PatchNode* changeRodToPatch(RodNode* n, Vector3 v);
	void translate(Vector3 v, bool withMesh = true);
	void unwrapBundleNodes();
	void unwrapBundleNode(QString nid);
	ScaffNode* wrapAsBundleNode(QVector<QString> nids, Vector3 v = Vector3(0, 0, 0));
	QVector<ScaffNode*> split(QString nid, Geom::Plane& plane);
	QVector<ScaffNode*> split(QString nid, QVector<Geom::Plane>& planes);

	// I/O
	void saveToFile(QString fname);
	void loadFromFile(QString fname);
	void exportWholeMesh(QString fname);

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


public:
	QString path;
	bool showAABB;

	// visual debugger
	VisualDebugger visDebug;
};

Q_DECLARE_METATYPE(QVector<Scaffold*>)