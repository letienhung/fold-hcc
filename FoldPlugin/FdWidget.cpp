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
	plugin->connect(ui->showDecomp, SIGNAL(stateChanged(int)), SLOT(setShowDecomp(int)));
	plugin->connect(ui->showKeyframe, SIGNAL(stateChanged(int)), SLOT(setShowKeyframe(int)));

	plugin->connect(ui->showCuboid, SIGNAL(stateChanged(int)), SLOT(setShowCuboid(int)));
	plugin->connect(ui->showScaffold, SIGNAL(stateChanged(int)), SLOT(setShowScaffold(int)));
	plugin->connect(ui->showMesh, SIGNAL(stateChanged(int)), SLOT(setShowMesh(int)));
	plugin->connect(ui->showAABB, SIGNAL(stateChanged(int)), SLOT(setShowAABB(int)));
	plugin->connect(ui->showAFS, SIGNAL(stateChanged(int)), SLOT(setShowAFS(int)));

	// foldabilize
	plugin->f_manager->connect(ui->sqzV, SIGNAL(currentIndexChanged(QString)), SLOT(setSqzV(QString)));
	plugin->f_manager->connect(ui->nbSplits, SIGNAL(valueChanged(int)), SLOT(setNbSplits(int)));
	plugin->f_manager->connect(ui->nbChunks, SIGNAL(valueChanged(int)), SLOT(setNbChunks(int)));
	plugin->f_manager->connect(ui->thickness, SIGNAL(valueChanged(double)), SLOT(setThickness(double)));
	plugin->f_manager->connect(ui->foldabilize, SIGNAL(clicked()), SLOT(foldabilize()));
	plugin->connect(ui->exportVector, SIGNAL(clicked()), SLOT(exportSVG()));
	plugin->f_manager->connect(ui->exportStat, SIGNAL(clicked()), SLOT(exportStat()));
	plugin->connect(ui->exportPNG, SIGNAL(clicked()), SLOT(exportPNG()));

	// decompose
	plugin->f_manager->connect(ui->decompose, SIGNAL(clicked()), SLOT(decompose()));
	this->connect(plugin->f_manager, SIGNAL(DcGraphsChanged(QStringList)), SLOT(setDcGraphList(QStringList)));
	this->connect(plugin->f_manager, SIGNAL(blocksChanged(QStringList)), SLOT(setBlockList(QStringList)));
	this->connect(plugin->f_manager, SIGNAL(chainsChanged(QStringList)), SLOT(setChainList(QStringList)));
	this->connect(ui->dcList, SIGNAL(itemSelectionChanged()), SLOT(selectDcGraph()));
	this->connect(ui->blockList, SIGNAL(itemSelectionChanged()), SLOT(selectBlock()));
	this->connect(ui->chainList, SIGNAL(itemSelectionChanged()), SLOT(selectChain()));
	plugin->f_manager->connect(this, SIGNAL(dcGraphSelectionChanged(QString)), SLOT(selectDcGraph(QString)));
	plugin->f_manager->connect(this, SIGNAL(blockSelectionChanged(QString)), SLOT(selectBlock(QString)));
	plugin->f_manager->connect(this, SIGNAL(chainSelectionChanged(QString)), SLOT(selectChain(QString)));
	plugin->f_manager->connect(ui->clearDcList, SIGNAL(clicked()), SLOT(clearDcGraphs()));

	// selected block
	this->connect(plugin->f_manager, SIGNAL(solutionsChanged(int)), SLOT(setSolutionList(int)));
	this->connect(ui->slnList, SIGNAL(itemSelectionChanged()), SLOT(selectSolution()));
	plugin->f_manager->connect(this, SIGNAL(solutionSelectionChanged(int)), SLOT(selectSolution(int)));
	plugin->f_manager->connect(ui->exportCollFOG, SIGNAL(clicked()), SLOT(exportCollFOG()));

	// key frames
	plugin->f_manager->connect(ui->nbKeyframes, SIGNAL(valueChanged(int)), SLOT(setNbKeyframes(int)));
	plugin->f_manager->connect(ui->genKeyframes, SIGNAL(clicked()), SLOT(generateKeyframes()));
	this->connect(plugin->f_manager, SIGNAL(keyframesChanged(int)), SLOT(setKeyframeSlider(int)));
	plugin->f_manager->connect(ui->keyframeSlider, SIGNAL(valueChanged(int)), SLOT(selectKeyframe(int)));
	plugin->connect(ui->exportCurrent, SIGNAL(clicked()), SLOT(exportCurrent()));
	plugin->connect(ui->exportAllObj, SIGNAL(clicked()), SLOT(exportAllObj()));
	
	// color
	plugin->connect(ui->assignColor, SIGNAL(clicked()), SLOT(showColorDialog()));
	plugin->connect(ui->colorMasterSlave, SIGNAL(clicked()), SLOT(colorMasterSlave()));

	// snapshot
	plugin->connect(ui->snapshot, SIGNAL(clicked()), SLOT(saveSnapshot()));
	plugin->connect(ui->snapshotAll, SIGNAL(clicked()), SLOT(saveSnapshotAll()));

	// hide
	plugin->connect(ui->hideSelNodes, SIGNAL(clicked()), SLOT(hideSelectedNodes()));
	plugin->connect(ui->unhideAllNodes, SIGNAL(clicked()), SLOT(unhideAllNodes()));
	plugin->connect(ui->hideEdgeRods, SIGNAL(clicked()), SLOT(hideEdgeRods()));

	// threshold
	plugin->f_manager->connect(ui->connThrRation, SIGNAL(valueChanged(double)), SLOT(setConnThrRatio(double)));
	plugin->f_manager->connect(ui->aabbX, SIGNAL(valueChanged(double)), SLOT(setAabbX(double)));
	plugin->f_manager->connect(ui->aabbY, SIGNAL(valueChanged(double)), SLOT(setAabbY(double)));
	plugin->f_manager->connect(ui->aabbZ, SIGNAL(valueChanged(double)), SLOT(setAabbZ(double)));
	plugin->f_manager->connect(ui->costWeight, SIGNAL(valueChanged(double)), SLOT(setCostWeight(double)));

	// test
	plugin->connect(ui->test1, SIGNAL(clicked()), SLOT(test1()));
	plugin->connect(ui->test2, SIGNAL(clicked()), SLOT(test2()));
}

void FdWidget::forceShowKeyFrame()
{
	ui->showKeyframe->setChecked(true);
	ui->keyframeSlider->setValue(0);
}

FdWidget::~FdWidget()
{
    delete ui;
}

void FdWidget::setDcGraphList( QStringList labels )
{
	ui->dcList->clear();
	ui->dcList->addItems(labels);
}

void FdWidget::setBlockList( QStringList labels )
{
	ui->blockList->clear();
	ui->blockList->addItems(labels);
}

void FdWidget::setChainList( QStringList labels )
{
	ui->chainList->clear();
	ui->chainList->addItems(labels);
}

void FdWidget::setKeyframeSlider( int N )
{
	ui->keyframeSlider->setMaximum(N);
	ui->nbKeyframeSlider->setText(QString::number(N));

	if(N > 1) forceShowKeyFrame();
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
	QList<QListWidgetItem *> selItems = ui->dcList->selectedItems();

	if (!selItems.isEmpty())
	{
		emit(dcGraphSelectionChanged(selItems.front()->text()));
	}
}

void FdWidget::selectBlock()
{
	QList<QListWidgetItem *> selItems = ui->blockList->selectedItems();

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

