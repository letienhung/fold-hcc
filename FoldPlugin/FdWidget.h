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
    explicit FdWidget(FdPlugin *f, QWidget *parent = 0);
    ~FdWidget();

private:
    Ui::FdWidget *ui;
	FdPlugin *fold;

public slots:
	void createScaffold();
	void fitCuboid();
	void setScaffoldName(QString name);
};

