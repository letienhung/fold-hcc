#pragma once

#include <QWidget>
#include "Foldabilizer.h"

namespace Ui {
class FoldabilizerWidget;
}

class FoldabilizerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FoldabilizerWidget(Foldabilizer *f, QWidget *parent = 0);
    ~FoldabilizerWidget();

private:
    Ui::FoldabilizerWidget *ui;
	Foldabilizer *fold;
};
