#pragma once
#include <QObject>

#include "UtilityGlobal.h"
#include "ScoffoldGraph.h"

class GraphManager : public QObject
{
	Q_OBJECT

public:
    GraphManager();

private:
	ScoffoldGraph* scoffold;

public slots:
	void createScoffold(SurfaceMeshModel * entireMesh);
};


