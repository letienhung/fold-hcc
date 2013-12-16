#pragma once
#include <QObject>

#include "UtilityGlobal.h"
#include "FdGraph.h"

class GraphManager : public QObject
{
	Q_OBJECT

public:
    GraphManager();
	~GraphManager();

public:
	SurfaceMeshModel* entireMesh;
	FdGraphPtr scaffold;

public slots:
	// setter
	void setMesh(Model* model);
	void setScaffold(FdGraphPtr fdg);

	// graph
	void createScaffold(int method);
	void saveScaffold();
	void loadScaffold();

	// node
	void refitSelectedNodes(int method);
	void changeTypeOfSelectedNodes();

	// link
	void linkSelectedNodes();

	// visualization
	void showCuboids(int state);
	void showScaffold(int state);
	void showMeshes(int state);
	void showAABB(int state);

	// test
	void test();

signals:
	void sceneSettingsChanged();
	void scaffoldModified();
	void scaffoldChanged(QString name);
	void message(QString msg);
};


