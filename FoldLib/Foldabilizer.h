#include "FdGraph.h"
#include "LayerModel.h"
#include "Numeric.h"
#include <QObject>

class Foldabilizer : public QObject
{
	Q_OBJECT

public:
    Foldabilizer();

public:
	FdGraph* scaffold; // weak reference to input scaffold

	int pushAxis;
	int selectedId;
	QVector<LayerModel*> layerModels;

public:
	FdGraph* selectedScaffold();

public slots:
	void setScaffold(FdGraph* fdg);
	void setPushAxis(int aid);
	void fold();
};

