#pragma once
#include <QObject>

#include "UtilityGlobal.h"
#include "FdGraph.h"

class GraphManager : public QObject
{
	Q_OBJECT

public:
    GraphManager();

private:
	FdGraph* scoffold;

public slots:
	void createScoffold(SurfaceMeshModel * entireMesh);
};


