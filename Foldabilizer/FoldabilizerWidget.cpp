#include "FoldabilizerWidget.h"
#include "ui_FoldabilizerWidget.h"

FoldabilizerWidget::FoldabilizerWidget(Foldabilizer *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FoldabilizerWidget)
{
    ui->setupUi(this);

	fold = f;

	// signal and slots
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

	// optimization: jump
	this->connect(ui->nbSigma, SIGNAL(valueChanged(int)), SLOT(updateMHOptimizerPara()));
	this->connect(ui->typeOne, SIGNAL(valueChanged(double)), SLOT(updateMHOptimizerPara()));
	this->connect(ui->typeTwo, SIGNAL(valueChanged(double)), SLOT(updateMHOptimizerPara()));
	this->connect(ui->switchHinge, SIGNAL(valueChanged(double)), SLOT(updateMHOptimizerPara()));
	this->connect(ui->useHot, SIGNAL(valueChanged(double)), SLOT(updateMHOptimizerPara()));

	this->connect(ui->alwaysAccept, SIGNAL(stateChanged (int)), SLOT(updateMHOptimizerPara()));
	this->connect(ui->distWeight, SIGNAL(valueChanged(double)), SLOT(updateMHOptimizerPara()));
	this->connect(ui->temprature, SIGNAL(valueChanged(int)), SLOT(updateMHOptimizerPara()));

	this->connect(ui->targetV, SIGNAL(valueChanged(int)), SLOT(updateMHOptimizerPara()));
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
	opt->nbSigma = ui->nbSigma->value();
	opt->setTypeProb(ui->typeOne->value(), ui->typeTwo->value());
	opt->switchHingeProb = ui->switchHinge->value();
	opt->useHotProb = ui->useHot->value();

	// accept
	opt->distWeight = ui->distWeight->value();
	opt->temperature = ui->temprature->value();
	opt->alwaysAccept = ui->alwaysAccept->isChecked();

	// target
	opt->targetV = ui->targetV->value() / 100.0;
	fold->stepsPerJump = ui->stepsPerJump->value();
}

