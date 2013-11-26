#include "FuiWidget.h"
#include "ui_FuiWidget.h"

FuiWidget::FuiWidget(FoldabilizerUi *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FuiWidget)
{
    ui->setupUi(this);
	plugin = f;

	// signals
	//plugin->connect(ui->sayHi, SIGNAL(clicked()), SLOT(sayHi()));
	plugin->connect(ui->loadButton, SIGNAL(clicked()), SLOT(loadLCC()));
	plugin->connect(ui->saveButton, SIGNAL(clicked()), SLOT(saveLCC()));
	plugin->connect(ui->showLCC, SIGNAL(stateChanged(int)), SLOT(showLCC(int)));
	plugin->connect(ui->createBoxBtn, SIGNAL(clicked()), SLOT(createBox()));
}

FuiWidget::~FuiWidget()
{
    if(ui)
		delete ui;
}

bool FuiWidget::ifuseAABB()
{
	return ui->useAABB->isChecked(); 
}

bool FuiWidget::ifshowLCC()
{
	return ui->showLCC->isChecked();
}

bool FuiWidget::ifshowModel()
{
	return ui->showModel->isChecked();
}
