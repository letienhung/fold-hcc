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

private:
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

	// update parameters
	void updateParameters();

public slots:
	/// Main pipeline	
	// input
	void setInputScaffold(Scaffold* fdg);

	// parameters
	void setSqzV (QString sqzV_str);
	void setNbSplits(int N);
	void setNbChunks(int N);
	void setThickness(double thk);
	void setConnThrRatio(double thr);
	void setAabbX(double x);
	void setAabbY(double y);
	void setAabbZ(double z);
	void setCostWeight(double w);

	// decompose
	void decompose();

	// fold
	void foldabilize();

	// keyframes
	void setNbKeyframes(int N);
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