#include "FdWidget.h"
#include "ui_FdWidget.h"

FdWidget::FdWidget(FdPlugin *fp, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FdWidget)
{
    ui->setupUi(this);
	plugin = fp;

	// connections
	this->connect(ui->DcList, SIGNAL(itemSelectionChanged()), SLOT(selectDcGraph()));
	this->connect(ui->layerList, SIGNAL(itemSelectionChanged()), SLOT(selectLayer()));
	this->connect(ui->chainList, SIGNAL(itemSelectionChanged()), SLOT(selectChain()));
	this->connect(ui->keyframeList, SIGNAL(itemSelectionChanged()), SLOT(selectKeyframe()));

	// creation and refine
	this->connect(plugin->g_manager, SIGNAL(scaffoldChanged(FdGraph*)), SLOT(setScaffold(FdGraph*)));
	plugin->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(resetMesh()));
	plugin->g_manager->connect(ui->fitMethod, SIGNAL(currentIndexChanged(int)), SLOT(setFitMethod(int)));
	plugin->g_manager->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(createScaffold()));
	plugin->g_manager->connect(ui->refitMethod, SIGNAL(currentIndexChanged(int)), SLOT(setRefitMethod(int)));
	plugin->g_manager->connect(ui->fitCuboid, SIGNAL(clicked()), SLOT(refitNodes()));
	plugin->g_manager->connect(ui->changeCuboidType, SIGNAL(clicked()), SLOT(changeNodeType()));
	plugin->g_manager->connect(ui->addLink, SIGNAL(clicked()), SLOT(linkNodes()));

	// save and load
	plugin->g_manager->connect(ui->saveScaffold, SIGNAL(clicked()), SLOT(saveScaffold()));
	plugin->g_manager->connect(ui->loadScaffold, SIGNAL(clicked()), SLOT(loadScaffold()));

	// fold
	plugin->f_manager->connect(ui->pushDirection, SIGNAL(currentIndexChanged(int)), SLOT(foldAlongAxis(int)));
	plugin->f_manager->connect(ui->createDc, SIGNAL(clicked()), SLOT(createDcGraphs()));

	this->connect(plugin->f_manager, SIGNAL(DcGraphsChanged(QStringList)), SLOT(setDcGraphList(QStringList)));
	this->connect(plugin->f_manager, SIGNAL(layersChanged(QStringList)), SLOT(setLayerList(QStringList)));
	this->connect(plugin->f_manager, SIGNAL(chainsChanged(QStringList)), SLOT(setChainList(QStringList)));
	this->connect(plugin->f_manager, SIGNAL(keyframesChanged(int)), SLOT(setKeyframeList(int)));
	plugin->f_manager->connect(this, SIGNAL(dcGraphSelectionChanged(QString)), SLOT(selectDcGraph(QString)));
	plugin->f_manager->connect(this, SIGNAL(layerSelectionChanged(QString)), SLOT(selectLayer(QString)));
	plugin->f_manager->connect(this, SIGNAL(chainSelectionChanged(QString)), SLOT(selectChain(QString)));
	plugin->f_manager->connect(this, SIGNAL(keyframeSelectionChanged(int)), SLOT(selectKeyframe(int)));

	plugin->f_manager->connect(ui->foldLayer, SIGNAL(clicked()), SLOT(foldSelLayer()));
	plugin->f_manager->connect(ui->snapshotTime, SIGNAL(valueChanged(double)), SLOT(snapshotSelLayer(double)));
	plugin->f_manager->connect(ui->exportFOG, SIGNAL(clicked()), SLOT(exportFOG()));
	plugin->f_manager->connect(ui->fold, SIGNAL(clicked()), SLOT(foldAll()));
	plugin->f_manager->connect(ui->generateKeyframes, SIGNAL(clicked()), SLOT(generateFdKeyFrames()));

	// visualization
	plugin->connect(ui->showKeyframe, SIGNAL(stateChanged(int)), SLOT(showKeyframe(int)));
	plugin->connect(ui->showFolded, SIGNAL(stateChanged(int)), SLOT(showFolded(int)));
	plugin->connect(ui->showCuboids, SIGNAL(stateChanged(int)), SLOT(showCuboid(int)));
	plugin->connect(ui->showScaffold, SIGNAL(stateChanged(int)), SLOT(showScaffold(int)));
	plugin->connect(ui->showMesh, SIGNAL(stateChanged(int)), SLOT(showMesh(int)));
	plugin->connect(ui->showAABB, SIGNAL(stateChanged(int)), SLOT(showAABB(int)));

	// export
	plugin->connect(ui->exportCurrent, SIGNAL(clicked()), SLOT(exportCurrent()));

	// test
	plugin->connect(ui->test1, SIGNAL(clicked()), SLOT(test1()));
	plugin->connect(ui->test2, SIGNAL(clicked()), SLOT(test2()));
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
	ui->DcList->clear();
	ui->DcList->addItems(labels);
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

void FdWidget::setKeyframeList( int N )
{
	ui->keyframeList->clear();
	QStringList labels;
	for (int i = 0; i < N; i++)
		labels << QString::number(i);

	ui->keyframeList->addItems(labels);
}

void FdWidget::selectDcGraph()
{
	QList<QListWidgetItem *> selItems = ui->DcList->selectedItems();

	if (!selItems.isEmpty())
	{
		emit(dcGraphSelectionChanged(selItems.front()->text()));
	}
}

void FdWidget::selectLayer()
{
	QList<QListWidgetItem *> selItems = ui->layerList->selectedItems();

	if (!selItems.isEmpty())
	{
		emit(layerSelectionChanged(selItems.front()->text()));
	}
}

void FdWidget::selectChain()
{
	QList<QListWidgetItem *> selItems = ui->chainList->selectedItems();

	if (!selItems.isEmpty())
	{
		emit(chainSelectionChanged(selItems.front()->text()));
	}
}

void FdWidget::selectKeyframe()
{
	QList<QListWidgetItem *> selItems = ui->keyframeList->selectedItems();

	if (!selItems.isEmpty())
	{
		int idx = selItems.front()->text().toInt();
		emit(keyframeSelectionChanged(idx));
	}
}
