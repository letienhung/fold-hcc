#pragma once
#include <QObject>

#include "UtilityGlobal.h"
#include "FdGraph.h"

class GraphManager : public QObject
{
	Q_OBJECT

public:
    GraphManager();
	SurfaceMeshModel* entireMesh;
	FdGraph* scaffold;

private:
	QMap<QString, Geom::Box> boxMap;

public slots:
	void setMesh(Model* model);

	void createScaffold(bool doFitting = true);
	void saveScaffold();
	void loadScaffold();

	void showCuboids(int state);
	void showScaffold(int state);

signals:
	void sceneSettingsChanged();
	void scaffoldChanged();
	void message(QString msg);
};


