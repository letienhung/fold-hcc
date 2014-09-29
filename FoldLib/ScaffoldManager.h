#pragma once
#include <QObject>

#include "UtilityGlobal.h"
#include "Scaffold.h"
#include "FdUtility.h"

class ScaffoldManager : public QObject
{
	Q_OBJECT

public:
    ScaffoldManager();
	~ScaffoldManager();

public:
	SurfaceMeshModel* entireMesh;
	Scaffold* scaffold;

	// ui 
	BOX_FIT_METHOD fitMethod;
	BOX_FIT_METHOD refitMethod;

public slots:
	// creation
	void setMesh(SurfaceMeshModel* mesh);
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


