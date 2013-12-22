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
	void selectLyGraph(QListWidgetItem* item);
	void selectLayer(QListWidgetItem* item);

	// to Ui
	void setScaffold(FdGraph* fdg);
	void setLyGraphList(QStringList labels);
	void setLayerList(QStringList labels);

signals:
	void lyGraphSelectionChanged(QString id);
	void layerSelectionChanged(QString id);
};

