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
	void selectKeyframe();
	void identifyMasters();
	void genKeyframes();

	// to Ui
	void setScaffold(FdGraph* fdg);
	void setDcGraphList(QStringList labels);
	void setLayerList(QStringList labels);
	void setChainList(QStringList labels);
	void setKeyframeList(int N);

signals:
	void dcGraphSelectionChanged(QString id);
	void blockSelectionChanged(QString id);
	void chainSelectionChanged(QString id);
	void keyframeSelectionChanged(int idx);
};

