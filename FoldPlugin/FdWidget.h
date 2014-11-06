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

public:
	Ui::FdWidget *ui;

private:
	FdPlugin *plugin;

public slots:
	// from Ui
	void selectUnit();
	void selectChain();

	// to Ui
	void setUnitList(QStringList labels);
	void setChainList(QStringList labels);
	void setKeyframeSlider(int N);
	void onKeyframeSliderValueChanged();
	void onKeyframTimeChanged();
	
	void forceShowKeyFrame();

signals:
	void unitSelectionChanged(QString id);
	void chainSelectionChanged(QString id);
	void keyframeSelectionChanged(int idx);
};

