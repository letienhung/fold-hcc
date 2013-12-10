#include "FoldabilizerWidget.h"
#include "ui_FoldabilizerWidget.h"

FoldabilizerWidget::FoldabilizerWidget(Foldabilizer *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FoldabilizerWidget)
{
    ui->setupUi(this);
	fold = f;

	// scaffold
	fold->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(resetMesh()));
	fold->connect(ui->loadScaffold, SIGNAL(clicked()), SLOT(resetMesh()));

	fold->g_manager->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(createScaffold()));
	fold->g_manager->connect(ui->saveScaffold, SIGNAL(clicked()), SLOT(saveScaffold()));
	fold->g_manager->connect(ui->loadScaffold, SIGNAL(clicked()), SLOT(loadScaffold()));

	// visualization
	fold->g_manager->connect(ui->showCuboids, SIGNAL(stateChanged(int)), SLOT(showCuboids(int)));
	fold->g_manager->connect(ui->showScaffold, SIGNAL(stateChanged(int)), SLOT(showScaffold(int)));

	fold->connect(ui->test, SIGNAL(clicked()), SLOT(test()));
}


FoldabilizerWidget::~FoldabilizerWidget()
{
    delete ui;
}
