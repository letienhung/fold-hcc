#pragma once

#include <QWidget>

#include "FoldabilizerUi.h"

namespace Ui {
class FuiWidget;
}

class FuiWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FuiWidget(FoldabilizerUi *fui, QWidget *parent = 0);
    ~FuiWidget();

//public SLOT: 
//	void displayModel(int state);
//	void displayLCC(int state);

public:
	bool ifuseAABB();
	bool ifshowModel();
	bool ifshowLCC();

private:
    Ui::FuiWidget *ui;
	FoldabilizerUi *plugin;
};

