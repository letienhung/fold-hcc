#pragma once

#include <QWidget>
#include "FdPlugin.h"
#include "QListWidgetItem"

namespace Ui {
class FdWidget;
}

class FdWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FdWidget(FdPlugin *fp, QWidget *parent = 0);
    ~FdWidget();

private:
    Ui::FdWidget *ui;
	FdPlugin *plugin;

public slots:
	// from Ui
	void selectDcGraph();
	void selectBlock();
	void selectChain();
	void selectSolution();

	// to Ui
	void setDcGraphList(QStringList labels);
	void setBlockList(QStringList labels);
	void setChainList(QStringList labels);
	void setKeyframeSlider(int N);
	void setSolutionList(int N);
	
signals:
	void dcGraphSelectionChanged(QString id);
	void blockSelectionChanged(QString id);
	void chainSelectionChanged(QString id);
	void keyframeSelectionChanged(int idx);
	void solutionSelectionChanged(int idx);
};

