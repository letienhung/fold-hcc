#include "FoldabilizerWidget.h"
#include "ui_FoldabilizerWidget.h"

FoldabilizerWidget::FoldabilizerWidget(Foldabilizer *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FoldabilizerWidget)
{
    ui->setupUi(this);
	fold = f;

	fold->connect(ui->test, SIGNAL(clicked()), SLOT(test()));
}


FoldabilizerWidget::~FoldabilizerWidget()
{
    delete ui;
}
