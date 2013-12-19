#include "FdWidget.h"
#include "ui_FdWidget.h"

FdWidget::FdWidget(FdPlugin *fp, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FdWidget)
{
    ui->setupUi(this);
	plugin = fp;

	// creation and refine
	plugin->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(resetMesh()));
	plugin->g_manager->connect(ui->fitMethod, SIGNAL(currentIndexChanged(int)), SLOT(setFitMethod(int)));
	plugin->g_manager->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(createScaffold()));
	plugin->g_manager->connect(ui->refitMethod, SIGNAL(currentIndexChanged(int)), SLOT(setRefitMethod(int)));
	plugin->g_manager->connect(ui->fitCuboid, SIGNAL(clicked()), SLOT(fitCuboid()));
	plugin->g_manager->connect(ui->changeCuboidType, SIGNAL(clicked()), SLOT(changeNodeType()));
	plugin->g_manager->connect(ui->addLink, SIGNAL(clicked()), SLOT(linkNodes()));

	// save and load
	plugin->g_manager->connect(ui->saveScaffold, SIGNAL(clicked()), SLOT(saveScaffold()));
	plugin->g_manager->connect(ui->loadScaffold, SIGNAL(clicked()), SLOT(loadScaffold()));

	// fold
	plugin->fold->connect(ui->pushDirection, SIGNAL(currentIndexChanged(int)), SLOT(setDirection(int)));
	plugin->fold->connect(ui->fold, SIGNAL(clicked()), SLOT(fold()));
	plugin->connect(ui->showFolded, SIGNAL(stateChanged(int)), SLOT(showFolded(int)));

	// visualization
	plugin->connect(ui->showCuboids, SIGNAL(stateChanged(int)), SLOT(showCuboid(int)));
	plugin->connect(ui->showScaffold, SIGNAL(stateChanged(int)), SLOT(showScaffold(int)));
	plugin->connect(ui->showMesh, SIGNAL(stateChanged(int)), SLOT(showMesh(int)));
	plugin->connect(ui->showAABB, SIGNAL(stateChanged(int)), SLOT(showAABB(int)));

	// test
	plugin->connect(ui->test, SIGNAL(clicked()), SLOT(test()));
}


FdWidget::~FdWidget()
{
    delete ui;
}

void FdWidget::setScaffold(FdGraph* fdg)
{
	QString path = "./";
	if(fdg) path += fdg->path.section("/", -3, -1);
	ui->fdPath->setText(path);
}
