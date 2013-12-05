#include "FoldabilizerWidget.h"
#include "ui_FoldabilizerWidget.h"

FoldabilizerWidget::FoldabilizerWidget(Foldabilizer *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FoldabilizerWidget)
{
    ui->setupUi(this);
	fold = f;

	// connect to plugin
	this->connect(fold->hccManager, SIGNAL(activeHccChanged()), SLOT(checkAllHinges()));
	
	// creating shapes
	fold->hccManager->connect(ui->createI,		SIGNAL(clicked()), SLOT(makeI()));
	fold->hccManager->connect(ui->createT,		SIGNAL(clicked()), SLOT(makeT()));
	fold->hccManager->connect(ui->createL,		SIGNAL(clicked()), SLOT(makeL()));
	fold->hccManager->connect(ui->createX,		SIGNAL(clicked()), SLOT(makeX()));
	fold->hccManager->connect(ui->createSharp,	SIGNAL(clicked()), SLOT(makeSharp()));
	fold->hccManager->connect(ui->createO,		SIGNAL(clicked()), SLOT(makeO()));
	fold->hccManager->connect(ui->createO_2,	SIGNAL(clicked()), SLOT(makeO_2()));
	fold->hccManager->connect(ui->createBox,	SIGNAL(clicked()), SLOT(makeBox()));

	this->connect(ui->createU,		SIGNAL(clicked()), SLOT(createU()));
	this->connect(ui->createChair,	SIGNAL(clicked()), SLOT(createChair()));
	this->connect(ui->createShelf,	SIGNAL(clicked()), SLOT(createShelf()));
	fold->connect(ui->load_hcc,		SIGNAL(clicked()), SLOT(loadGraph()));

	// hinge detector
	this->connect(ui->eeHinge,		SIGNAL(stateChanged(int)), SLOT(updateHinges()));
	this->connect(ui->efHinge,		SIGNAL(stateChanged(int)), SLOT(updateHinges()));
	this->connect(ui->ffHinge,		SIGNAL(stateChanged(int)), SLOT(updateHinges()));

	// optimization: jump
	this->connect(ui->alwaysAccept, SIGNAL(stateChanged(int)),		SLOT(updateMHOptimizerPara()));
	this->connect(ui->distWeight,	SIGNAL(valueChanged(double)),	SLOT(updateMHOptimizerPara()));
	this->connect(ui->temprature,	SIGNAL(valueChanged(int)),		SLOT(updateMHOptimizerPara()));
	this->connect(ui->resCollProb,	SIGNAL(valueChanged(double)),	SLOT(updateMHOptimizerPara()));
	this->connect(ui->targetV,		SIGNAL(valueChanged(double)),	SLOT(updateMHOptimizerPara()));

	this->connect(ui->jumpButton,	SIGNAL(clicked()), SLOT(jump()));
	fold->connect(ui->test,			SIGNAL(clicked()), SLOT(test()));
}


FoldabilizerWidget::~FoldabilizerWidget()
{
    delete ui;
}

void FoldabilizerWidget::createU()
{
	fold->hccManager->makeU(ui->ULeft->value(), ui->UMid->value(), ui->URight->value());
}

void FoldabilizerWidget::createChair()
{
	fold->hccManager->makeChair(ui->legLength->value());
}

void FoldabilizerWidget::createShelf()
{
	fold->hccManager->makeShelf(ui->nbLayers->value());
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
}

void FoldabilizerWidget::updateHinges()
{
	if (!fold->activeHcc()) return;

	bool ee = ui->eeHinge->isChecked();
	bool ef = ui->efHinge->isChecked();
	bool ff = ui->ffHinge->isChecked();

	// update hinges
	fold->activeHcc()->detectHinges(ee, ef, ff);
	fold->drawArea()->update();
}

void FoldabilizerWidget::checkAllHinges()
{
	ui->eeHinge->setCheckState(Qt::Checked);
	ui->efHinge->setCheckState(Qt::Checked);
	ui->ffHinge->setCheckState(Qt::Checked);
}

void FoldabilizerWidget::jump()
{
	fold->mhOptimizer->jump(ui->stepsPerJump->value());
}
