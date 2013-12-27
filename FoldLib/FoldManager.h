#include "FdGraph.h"
#include "DcGraph.h"
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

	int selId;
	QVector<DcGraph*> dcGraphs;

public:
	void createLayerGraphs(Vector3 pushDirect); 

	FdGraph* activeScaffold();
	DcGraph* getSelDcGraph();
	QStringList getDcGraphLabels();
	void updateLists();

	LayerGraph* getSelLayer();

public slots:
	void setScaffold(FdGraph* fdg);
	void setPushAxis(int aid);
	void createLayerGraphs();
	void selectDcGraph(QString id);
	void selectLayer(QString id);
	void selectChain(QString id);

	void fold();

signals:
	void selectionChanged();
	void lyGraphsChanged(QStringList labels);
	void layersChanged(QStringList labels);
	void chainsChanged(QStringList labels);
};

