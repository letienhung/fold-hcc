#pragma once

#include <QWidget>
#include "FdPlugin.h"

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
	// to Ui
	void setScaffold(FdGraph* fdg);
};

