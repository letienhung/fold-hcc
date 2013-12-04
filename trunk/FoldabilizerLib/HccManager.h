#pragma once
#include <QObject>
#include <UtilityGlobal.h>
#include "HccGraph.h"

class HccManager : public QObject
{
	Q_OBJECT

public:
	HccManager();
	~HccManager();

	// Active HCC
	HccGraph* hccGraph;
	HccGraph* activeHcc();

	// Visualize
	void draw();
	
	// File I/O
	void loadHCC(QString filename);

public slots:
	// Prepare data
	void makeI();
	void makeL();
	void makeT();
	void makeX();
	void makeSharp();
	void makeU(double uleft, double umid, double uright);
	void makeO();
	void makeO_2();
	void makeBox();
	void makeChair(double legL);

signals:
	void activeHccChanged();
};
