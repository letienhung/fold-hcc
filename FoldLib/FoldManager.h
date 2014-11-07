#pragma once

#include "Scaffold.h"
#include "DecScaff.h"
#include "Numeric.h"
#include <QObject>

class FoldManager final : public QObject
{
	Q_OBJECT

public:
    FoldManager();
	~FoldManager();

public:
	// input
	Scaffold* inputScaffold; 

	// decomposition
	DecScaff* shapeDec;

	// timing
	double elapsedTime;

	// update Ui
	void updateUnitList();
	void updateChainList();
	void updateKeyframeSlider();

	// getters
	Scaffold* activeScaffold();
	UnitScaff* getSelUnit();
	Scaffold* getSelKeyframe();

	// statistics
	void genStat();
	void exportStat();

public slots:
	/// Main pipeline	
	// input
	void setInputScaffold(Scaffold* fdg);

	// decompose
	void decompose();

	// fold
	void foldabilize();

	// keyframes
	void generateKeyframes();

	// output
	void exportResultMesh();

	// selection signal from Ui
	void selectUnit(QString id);
	void selectChain(QString id);
	void selectKeyframe(int idx);

signals:
	// notify others about changes
	void sceneChanged();
	void unitsChanged(QStringList labels);
	void chainsChanged(QStringList labels);
	void keyframesChanged(int N);
	void message(QString msg);
};