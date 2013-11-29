#include "FoldabilizerWidget.h"
#include "ui_FoldabilizerWidget.h"

FoldabilizerWidget::FoldabilizerWidget(Foldabilizer *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FoldabilizerWidget)
{
    ui->setupUi(this);

	fold = f;

	// connect to plugin
	this->connect(fold, SIGNAL(hccGraphChanged()), SLOT(checkAllHinges()));
	
	// creating shapes
	fold->connect(ui->createL, SIGNAL(clicked()), SLOT(createL()));
	fold->connect(ui->createI, SIGNAL(clicked()), SLOT(createI()));
	fold->connect(ui->createT, SIGNAL(clicked()), SLOT(createT()));
	fold->connect(ui->createX, SIGNAL(clicked()), SLOT(createX()));
	fold->connect(ui->createSharp, SIGNAL(clicked()), SLOT(createSharp()));
	fold->connect(ui->createO, SIGNAL(clicked()), SLOT(createO()));
	fold->connect(ui->load_hcc, SIGNAL(clicked()), SLOT(loadGraph()));

	this->connect(ui->createU, SIGNAL(clicked()), SLOT(createU()));
	this->connect(ui->createChair, SIGNAL(clicked()), SLOT(createChair()));

	// hinge detector
	this->connect(ui->eeHinge, SIGNAL(stateChanged(int)), SLOT(updateHinges()));
	this->connect(ui->efHinge, SIGNAL(stateChanged(int)), SLOT(updateHinges()));
	this->connect(ui->ffHinge, SIGNAL(stateChanged(int)), SLOT(updateHinges()));

	// optimization: jump
	this->connect(ui->alwaysAccept, SIGNAL(stateChanged (int)), SLOT(updateMHOptimizerPara()));
	this->connect(ui->distWeight, SIGNAL(valueChanged(double)), SLOT(updateMHOptimizerPara()));
	this->connect(ui->temprature, SIGNAL(valueChanged(int)), SLOT(updateMHOptimizerPara()));
	this->connect(ui->resCollProb, SIGNAL(valueChanged(double)), SLOT(updateMHOptimizerPara()));

	this->connect(ui->targetV, SIGNAL(valueChanged(double)), SLOT(updateMHOptimizerPara()));
	this->connect(ui->stepsPerJump, SIGNAL(valueChanged(int)), SLOT(updateMHOptimizerPara()));

	fold->connect(ui->jumpButton, SIGNAL(clicked()), SLOT(jump()));
	fold->connect(ui->test, SIGNAL(clicked()), SLOT(test()));
}


FoldabilizerWidget::~FoldabilizerWidget()
{
    delete ui;
}

void FoldabilizerWidget::createU()
{
	fold->createU(ui->ULeft->value(), ui->UMid->value(), ui->URight->value());
}

void FoldabilizerWidget::createChair()
{
	fold->createChair(ui->legLength->value());
}

void FoldabilizerWidget::updateMHOptimizerPara()
{
	MHOptimizer *opt = fold->mhOptimizer;
	if (!opt) return;

	// propose
	opt->resCollProb = ui->resCollProb->value();

	// accept
	opt->distWeight = ui->distWeight->value();
	opt->temperature = ui->temprature->value();
	opt->alwaysAccept = ui->alwaysAccept->isChecked();

	// target
	opt->targetVPerc = ui->targetV->value();
	fold->stepsPerJump = ui->stepsPerJump->value();
}

void FoldabilizerWidget::updateHinges()
{
	if (!fold->hccGraph) return;

	bool ee = ui->eeHinge->isChecked();
	bool ef = ui->efHinge->isChecked();
	bool ff = ui->ffHinge->isChecked();

	// update hinges
	fold->hccGraph->detectHinges(ee, ef, ff);
	fold->drawArea()->update();
}

void FoldabilizerWidget::checkAllHinges()
{
	ui->eeHinge->setCheckState(Qt::Checked);
	ui->efHinge->setCheckState(Qt::Checked);
	ui->ffHinge->setCheckState(Qt::Checked);
}