#pragma once

#include <QWidget>
#include "foldabilizer.h"

namespace Ui {
class foldem_widget;
}

class foldem_widget : public QWidget
{
    Q_OBJECT

public:
    explicit foldem_widget(foldabilizer *f, QWidget *parent = 0);
    ~foldem_widget();

private:
    Ui::foldem_widget *ui;
	foldabilizer *plugin;
};

