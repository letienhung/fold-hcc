#pragma once
#include <QObject>

#include "UtilityGlobal.h"
#include "Scaffold.h"
#include "FdUtility.h"

class ScaffManager final : public QObject
{
	Q_OBJECT

public:
    ScaffManager();
	~ScaffManager();

public:
	SurfaceMeshModel* wholeMesh;
	Scaffold* scaffold;

	// ui 
	BOX_FIT_METHOD fitMethod;
	BOX_FIT_METHOD refitMethod;

public slots:
	// creation
	void setMesh(Model* mesh);
	void setFitMethod(int method);
	void setRefitMethod(int method);
	void createScaffold();
	void refitNodes();
	void changeNodeType();
	void linkNodes();

	// I/O
	void saveScaffold();
	void loadScaffold();

	// test
	void test();

signals:
	void scaffoldModified();
	void scaffoldChanged(Scaffold* fdg);
	void message(QString msg);
};


