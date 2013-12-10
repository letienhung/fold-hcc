#pragma once
#include <QObject>

#include "UtilityGlobal.h"
#include "FdGraph.h"

class GraphManager : public QObject
{
	Q_OBJECT

public:
    GraphManager();
	FdGraph* scaffold;

public slots:
	void createScaffold(SurfaceMeshModel * entireMesh);
	void showCuboids(int state);
	void showScaffold(int state);

signals:
};


