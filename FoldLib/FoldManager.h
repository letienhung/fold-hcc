#include "FdGraph.h"
#include "LyGraph.h"
#include "Numeric.h"
#include <QObject>

class FoldManager : public QObject
{
	Q_OBJECT

public:
    FoldManager();
	~FoldManager();
	void clear();

public:
	FdGraph* scaffold; 
	int pushAxis; // X, Y, Z, ALL

	int selLyId;
	QVector<LyGraph*> lyGraphs;

public:
	FdGraph* activeScaffold();
	LyGraph* getSelectedLyGraph();
	void updateLists();
	void createLayerGraphs(Vector3 pushDirect); 

public slots:
	void setScaffold(FdGraph* fdg);
	void setPushAxis(int aid);
	void createLayerGraphs();
	void selectLyGraph(QString id);
	void selectLayer(QString id);

	void fold();

signals:
	void selectionChanged();
	void lyGraphsChanged(QStringList labels);
	void layersChanged(QStringList labels);
};

