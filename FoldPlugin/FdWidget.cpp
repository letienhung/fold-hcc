#include "FdWidget.h"
#include "ui_FdWidget.h"

FdWidget::FdWidget(FdPlugin *fp, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FdWidget)
{
    ui->setupUi(this);
	plugin = fp;

	// structural abstraction
	plugin->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(resetMesh()));
	plugin->g_manager->connect(ui->fitMethod, SIGNAL(currentIndexChanged(int)), SLOT(setFitMethod(int)));
	plugin->g_manager->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(createScaffold()));
	plugin->g_manager->connect(ui->refitMethod, SIGNAL(currentIndexChanged(int)), SLOT(setRefitMethod(int)));
	plugin->g_manager->connect(ui->fitCuboid, SIGNAL(clicked()), SLOT(refitNodes()));
	plugin->g_manager->connect(ui->changeCuboidType, SIGNAL(clicked()), SLOT(changeNodeType()));

	plugin->g_manager->connect(ui->saveScaffold, SIGNAL(clicked()), SLOT(saveScaffold()));
	plugin->g_manager->connect(ui->loadScaffold, SIGNAL(clicked()), SLOT(loadScaffold()));

	// visual options
	plugin->connect(ui->showKeyframe, SIGNAL(stateChanged(int)), SLOT(showKeyframe(int)));
	plugin->connect(ui->showFolded, SIGNAL(stateChanged(int)), SLOT(showFolded(int)));
	plugin->connect(ui->showCuboids, SIGNAL(stateChanged(int)), SLOT(showCuboid(int)));
	plugin->connect(ui->showScaffold, SIGNAL(stateChanged(int)), SLOT(showScaffold(int)));
	plugin->connect(ui->showMesh, SIGNAL(stateChanged(int)), SLOT(showMesh(int)));

	// foldabilize
	plugin->f_manager->connect(ui->sqzV, SIGNAL(currentIndexChanged(QString)), SLOT(setSqzV(QString)));
	plugin->f_manager->connect(ui->nbFolds, SIGNAL(valueChanged(int)), SLOT(setNbFolds(int)));
	plugin->f_manager->connect(ui->nbChunks, SIGNAL(valueChanged(int)), SLOT(setNbChunks(int)));
	plugin->f_manager->connect(ui->useThickness, SIGNAL(stateChanged(int)), SLOT(useThickness(int)));
	plugin->f_manager->connect(ui->thickness, SIGNAL(valueChanged(double)), SLOT(setThickness(double)));
	plugin->f_manager->connect(ui->foldabilize, SIGNAL(clicked()), SLOT(foldabilize()));

	// decompose
	plugin->f_manager->connect(ui->decompose, SIGNAL(clicked()), SLOT(decompose()));
	this->connect(plugin->f_manager, SIGNAL(DcGraphsChanged(QStringList)), SLOT(setDcGraphList(QStringList)));
	this->connect(plugin->f_manager, SIGNAL(blocksChanged(QStringList)), SLOT(setLayerList(QStringList)));
	this->connect(plugin->f_manager, SIGNAL(chainsChanged(QStringList)), SLOT(setChainList(QStringList)));
	this->connect(ui->DcList, SIGNAL(itemSelectionChanged()), SLOT(selectDcGraph()));
	this->connect(ui->layerList, SIGNAL(itemSelectionChanged()), SLOT(selectBlock()));
	this->connect(ui->chainList, SIGNAL(itemSelectionChanged()), SLOT(selectChain()));
	plugin->f_manager->connect(this, SIGNAL(dcGraphSelectionChanged(QString)), SLOT(selectDcGraph(QString)));
	plugin->f_manager->connect(this, SIGNAL(blockSelectionChanged(QString)), SLOT(selectBlock(QString)));
	plugin->f_manager->connect(this, SIGNAL(chainSelectionChanged(QString)), SLOT(selectChain(QString)));
	plugin->f_manager->connect(ui->clearDcList, SIGNAL(clicked()), SLOT(clearDcGraphs()));

	// selected block
	plugin->f_manager->connect(ui->foldbzSelBlock, SIGNAL(clicked()), SLOT(foldbzSelBlock()));
	this->connect(plugin->f_manager, SIGNAL(solutionsChanged(int)), SLOT(setSolutionList(int)));
	plugin->f_manager->connect(ui->exportCollFOG, SIGNAL(clicked()), SLOT(exportCollFOG()));
	this->connect(ui->slnList, SIGNAL(itemSelectionChanged()), SLOT(selectSolution()));
	plugin->f_manager->connect(this, SIGNAL(solutionSelectionChanged(int)), SLOT(selectSolution(int)));

	// key frames
	plugin->f_manager->connect(ui->nbKeyframes, SIGNAL(valueChanged(int)), SLOT(setNbKeyframes(int)));
	plugin->f_manager->connect(ui->genKeyframes, SIGNAL(clicked()), SLOT(generateKeyframes()));
	this->connect(plugin->f_manager, SIGNAL(keyframesChanged(int)), SLOT(setKeyframeSlider(int)));
	plugin->f_manager->connect(ui->keyframeSlider, SIGNAL(valueChanged(int)), SLOT(selectKeyframe(int)));
	plugin->connect(ui->exportCurrent, SIGNAL(clicked()), SLOT(exportCurrent()));

	// test
	plugin->connect(ui->test1, SIGNAL(clicked()), SLOT(test1()));
	plugin->connect(ui->test2, SIGNAL(clicked()), SLOT(test2()));
}


FdWidget::~FdWidget()
{
    delete ui;
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

void FdWidget::setKeyframeSlider( int N )
{
	ui->keyframeSlider->setMaximum(N);
}

void FdWidget::setSolutionList( int N )
{
	ui->slnList->clear();
	QStringList labels;
	for (int i = 0; i < N; i++)
		labels << QString::number(i);

	ui->slnList->addItems(labels);
}


void FdWidget::selectDcGraph()
{
	QList<QListWidgetItem *> selItems = ui->DcList->selectedItems();

	if (!selItems.isEmpty())
	{
		emit(dcGraphSelectionChanged(selItems.front()->text()));
	}
}

void FdWidget::selectBlock()
{
	QList<QListWidgetItem *> selItems = ui->layerList->selectedItems();

	if (!selItems.isEmpty())
	{
		emit(blockSelectionChanged(selItems.front()->text()));
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


void FdWidget::selectSolution()
{
	QList<QListWidgetItem *> selItems = ui->slnList->selectedItems();

	if (!selItems.isEmpty())
	{
		int idx = selItems.front()->text().toInt();
		emit(solutionSelectionChanged(idx));
	}
}
