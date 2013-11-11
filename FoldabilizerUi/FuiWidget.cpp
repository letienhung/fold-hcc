#include "FuiWidget.h"
#include "ui_FuiWidget.h"

FuiWidget::FuiWidget(FoldabilizerUi *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FuiWidget)
{
    ui->setupUi(this);
	plugin = f;

	// signals
	plugin->connect(ui->sayHi, SIGNAL(clicked()), SLOT(sayHi()));
}

FuiWidget::~FuiWidget()
{
    delete ui;
}
