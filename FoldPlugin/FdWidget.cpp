#include "FdWidget.h"
#include "ui_FdWidget.h"

FdWidget::FdWidget(FdPlugin *fp, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FdWidget)
{
    ui->setupUi(this);
	plugin = fp;

	// connections
	this->connect(ui->scaffoldList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(selectDcGraph(QListWidgetItem*)));
	this->connect(ui->layerList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(selectLayer(QListWidgetItem*)));
	this->connect(ui->chainList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(selectChain(QListWidgetItem*)));

	// creation and refine
	this->connect(plugin->g_manager, SIGNAL(scaffoldChanged(FdGraph*)), SLOT(setScaffold(FdGraph*)));
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
	plugin->f_manager->connect(ui->pushDirection, SIGNAL(currentIndexChanged(int)), SLOT(setPushAxis(int)));
	plugin->f_manager->connect(ui->createLayers, SIGNAL(clicked()), SLOT(createLayerGraphs()));

	this->connect(plugin->f_manager, SIGNAL(lyGraphsChanged(QStringList)), SLOT(setDcGraphList(QStringList)));
	this->connect(plugin->f_manager, SIGNAL(layersChanged(QStringList)), SLOT(setLayerList(QStringList)));
	this->connect(plugin->f_manager, SIGNAL(chainsChanged(QStringList)), SLOT(setChainList(QStringList)));
	plugin->f_manager->connect(this, SIGNAL(dcGraphSelectionChanged(QString)), SLOT(selectDcGraph(QString)));
	plugin->f_manager->connect(this, SIGNAL(layerSelectionChanged(QString)), SLOT(selectLayer(QString)));
	plugin->f_manager->connect(this, SIGNAL(chainSelectionChanged(QString)), SLOT(selectChain(QString)));

	plugin->f_manager->connect(ui->fold, SIGNAL(clicked()), SLOT(fold()));

	// visualization
	plugin->connect(ui->showFolded, SIGNAL(stateChanged(int)), SLOT(showFolded(int)));
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

void FdWidget::setDcGraphList( QStringList labels )
{
	ui->scaffoldList->clear();
	ui->scaffoldList->addItems(labels);
}

void FdWidget::setLayerList( QStringList labels )
{
	ui->layerList->clear();
	ui->layerList->addItems(labels);
}

void FdWidget::setChainList( QStringList labels )
{
	ui->chainList->clear();
	ui->chainList->addItems(labels);
}

void FdWidget::selectDcGraph( QListWidgetItem* item )
{
	emit(dcGraphSelectionChanged(item->text()));
}

void FdWidget::selectLayer( QListWidgetItem* item )
{
	emit(layerSelectionChanged(item->text()));
}

void FdWidget::selectChain( QListWidgetItem* item )
{
	emit(chainSelectionChanged(item->text()));
}
