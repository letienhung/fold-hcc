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

	int selectedId;
	QVector<LyGraph*> layerGraphs;

public:
	FdGraph* activeScaffold();

	void createLayerGraphs(Vector3 pushDirect); 

public slots:
	void setScaffold(FdGraph* fdg);
	void setPushAxis(int aid);
	void createLayerGraphs();


	void fold();
};

