#include "FoldabilizerWidget.h"
#include "ui_FoldabilizerWidget.h"

FoldabilizerWidget::FoldabilizerWidget(Foldabilizer *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FoldabilizerWidget)
{
    ui->setupUi(this);
	fold = f;

	// scaffold
	fold->connect(ui->test, SIGNAL(clicked()), SLOT(test()));
	fold->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(createScaffold()));

	// visualization
	fold->g_manager->connect(ui->showCuboids, SIGNAL(stateChanged(int)), SLOT(showCuboids(int)));
	fold->g_manager->connect(ui->showScaffold, SIGNAL(stateChanged(int)), SLOT(showScaffold(int)));
	fold->connect(ui->showCuboids, SIGNAL(stateChanged(int)), SLOT(updateScene()));
	fold->connect(ui->showScaffold, SIGNAL(stateChanged(int)), SLOT(updateScene()));
}


FoldabilizerWidget::~FoldabilizerWidget()
{
    delete ui;
}
