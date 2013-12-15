#include "FdWidget.h"
#include "ui_FdWidget.h"

FdWidget::FdWidget(FdPlugin *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FdWidget)
{
    ui->setupUi(this);
	fold = f;

	// creation and refine
	fold->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(resetMesh()));
	this->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(createScaffold()));

	this->connect(ui->fitCuboid, SIGNAL(clicked()), SLOT(fitCuboid()));
	fold->g_manager->connect(ui->changeCuboidType, SIGNAL(clicked()), SLOT(changeTypeOfSelectedNodes()));

	fold->g_manager->connect(ui->addLink, SIGNAL(clicked()), SLOT(linkSelectedNodes()));

	// save and load
	fold->g_manager->connect(ui->saveScaffold, SIGNAL(clicked()), SLOT(saveScaffold()));
	fold->g_manager->connect(ui->loadScaffold, SIGNAL(clicked()), SLOT(loadScaffold()));
	fold->connect(ui->loadScaffold, SIGNAL(clicked()), SLOT(resetMesh()));

	// visualization
	fold->g_manager->connect(ui->showCuboids, SIGNAL(stateChanged(int)), SLOT(showCuboids(int)));
	fold->g_manager->connect(ui->showScaffold, SIGNAL(stateChanged(int)), SLOT(showScaffold(int)));
	fold->g_manager->connect(ui->showMesh, SIGNAL(stateChanged(int)), SLOT(showMeshes(int)));
	fold->g_manager->connect(ui->showAABB, SIGNAL(stateChanged(int)), SLOT(showAABB(int)));

	// fold
	fold->connect(ui->test, SIGNAL(clicked()), SLOT(test()));
}


FdWidget::~FdWidget()
{
    delete ui;
}

void FdWidget::fitCuboid()
{
	if (fold->g_manager)
	{
		fold->g_manager->refitSelectedNodes(ui->fitMethod->currentIndex());
	}
}

void FdWidget::createScaffold()
{
	if (fold->g_manager)
	{
		fold->g_manager->createScaffold(ui->createMethod->currentIndex());
	}
}
