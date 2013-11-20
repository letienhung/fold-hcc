#include "FoldabilizerWidget.h"
#include "ui_FoldabilizerWidget.h"

FoldabilizerWidget::FoldabilizerWidget(Foldabilizer *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FoldabilizerWidget)
{
    ui->setupUi(this);

	fold = f;

	// signal and slots
	fold->connect(ui->createL, SIGNAL(clicked()), SLOT(createL()));
	fold->connect(ui->createI, SIGNAL(clicked()), SLOT(createI()));
	fold->connect(ui->createChair, SIGNAL(clicked()), SLOT(createChair()));
	fold->connect(ui->createT, SIGNAL(clicked()), SLOT(createT()));
	fold->connect(ui->createX, SIGNAL(clicked()), SLOT(createX()));
	fold->connect(ui->createSharp, SIGNAL(clicked()), SLOT(createSharp()));
	fold->connect(ui->createU, SIGNAL(clicked()), SLOT(createU()));
	fold->connect(ui->createO, SIGNAL(clicked()), SLOT(createO()));
	fold->connect(ui->load_hcc, SIGNAL(clicked()), SLOT(loadGraph()));
	fold->connect(ui->jumpButton, SIGNAL(clicked()), SLOT(jump()));
	fold->connect(ui->test, SIGNAL(clicked()), SLOT(test()));

	fold->connect(ui->legLength, SIGNAL(valueChanged(double)), SLOT(createChair(double)));

	fold->connect(ui->targetVolumePercentage, SIGNAL(valueChanged(int)), SLOT(setTargetVolumePercentage(int)));
	fold->connect(ui->linkProbability, SIGNAL(valueChanged(double)), SLOT(setLinkProbability(double)));
	fold->connect(ui->costWeight, SIGNAL(valueChanged(double)), SLOT(setCostWeight(double)));
	fold->connect(ui->temprature, SIGNAL(valueChanged(int)), SLOT(setTemprature(int)));
	fold->connect(ui->stepsPerJump, SIGNAL(valueChanged(int)), SLOT(setStepsPerJump(int)));
}


FoldabilizerWidget::~FoldabilizerWidget()
{
    delete ui;
}
