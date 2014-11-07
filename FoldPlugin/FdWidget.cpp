#include "FdWidget.h"
#include "ui_FdWidget.h"
#include "ParSingleton.h"

FdWidget::FdWidget(FdPlugin *fdp, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FdWidget)
{
    ui->setupUi(this);
	plugin = fdp;

	// structural abstraction
	plugin->s_manager->connect(ui->fitMethod, SIGNAL(currentIndexChanged(int)), SLOT(setFitMethod(int)));
	plugin->s_manager->connect(ui->createScaffold, SIGNAL(clicked()), SLOT(createScaffold()));
	plugin->s_manager->connect(ui->refitMethod, SIGNAL(currentIndexChanged(int)), SLOT(setRefitMethod(int)));
	plugin->s_manager->connect(ui->fitCuboid, SIGNAL(clicked()), SLOT(refitNodes()));
	plugin->s_manager->connect(ui->changeCuboidType, SIGNAL(clicked()), SLOT(changeNodeType()));

	plugin->s_manager->connect(ui->saveScaffold, SIGNAL(clicked()), SLOT(saveScaffold()));
	plugin->s_manager->connect(ui->loadScaffold, SIGNAL(clicked()), SLOT(loadScaffold()));

	// visual options
	ParSingleton* ps = ParSingleton::instance();
	ps->connect(ui->showDecomp, SIGNAL(stateChanged(int)), SLOT(setShowDecomp(int)));
	ps->connect(ui->showKeyframe, SIGNAL(stateChanged(int)), SLOT(setShowKeyframe(int)));

	ps->connect(ui->showCuboid, SIGNAL(stateChanged(int)), SLOT(setShowCuboid(int)));
	ps->connect(ui->showScaffold, SIGNAL(stateChanged(int)), SLOT(setShowScaffold(int)));
	ps->connect(ui->showMesh, SIGNAL(stateChanged(int)), SLOT(setShowMesh(int)));
	ps->connect(ui->showAABB, SIGNAL(stateChanged(int)), SLOT(setShowAABB(int)));

	// foldabilize
	ps->connect(ui->sqzV, SIGNAL(currentIndexChanged(QString)), SLOT(setSqzV(QString)));
	ps->connect(ui->nbSplits, SIGNAL(valueChanged(int)), SLOT(setNbSplits(int)));
	ps->connect(ui->nbChunks, SIGNAL(valueChanged(int)), SLOT(setNbChunks(int)));
	ps->connect(ui->thickness, SIGNAL(valueChanged(double)), SLOT(setThickness(double)));
	ps->connect(ui->connThrRatio, SIGNAL(valueChanged(double)), SLOT(setConnThrRatio(double)));
	ps->connect(ui->aabbX, SIGNAL(valueChanged(double)), SLOT(setAabbX(double)));
	ps->connect(ui->aabbY, SIGNAL(valueChanged(double)), SLOT(setAabbY(double)));
	ps->connect(ui->aabbZ, SIGNAL(valueChanged(double)), SLOT(setAabbZ(double)));
	ps->connect(ui->costWeight, SIGNAL(valueChanged(double)), SLOT(setCostWeight(double)));

	plugin->f_manager->connect(ui->foldabilize, SIGNAL(clicked()), SLOT(foldabilize()));

	// export
	plugin->connect(ui->exportVector, SIGNAL(clicked()), SLOT(exportSVG()));
	plugin->connect(ui->exportPNG, SIGNAL(clicked()), SLOT(exportPNG()));

	// decompose
	plugin->f_manager->connect(ui->decompose, SIGNAL(clicked()), SLOT(decompose()));
	this->connect(plugin->f_manager, SIGNAL(unitsChanged(QStringList)), SLOT(setUnitList(QStringList)));
	this->connect(plugin->f_manager, SIGNAL(chainsChanged(QStringList)), SLOT(setChainList(QStringList)));
	this->connect(ui->unitList, SIGNAL(itemSelectionChanged()), SLOT(selectUnit()));
	this->connect(ui->chainList, SIGNAL(itemSelectionChanged()), SLOT(selectChain()));
	plugin->f_manager->connect(this, SIGNAL(unitSelectionChanged(QString)), SLOT(selectUnit(QString)));
	plugin->f_manager->connect(this, SIGNAL(chainSelectionChanged(QString)), SLOT(selectChain(QString)));

	// key frames
	ps->connect(ui->nbKeyframes, SIGNAL(valueChanged(int)), SLOT(setNbKeyframes(int)));
	plugin->f_manager->connect(ui->genKeyframes, SIGNAL(clicked()), SLOT(generateKeyframes()));
	this->connect(plugin->f_manager, SIGNAL(keyframesChanged(int)), SLOT(setKeyframeSlider(int)));
	this->connect(ui->keyframeSlider, SIGNAL(valueChanged(int)), SLOT(onKeyframeSliderValueChanged()));
	this->connect(ui->keyframeTime, SIGNAL(valueChanged(double)), SLOT(onKeyframTimeChanged()));
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

void FdWidget::setUnitList( QStringList labels )
{
	ui->unitList->clear();
	ui->unitList->addItems(labels);
}

void FdWidget::setChainList( QStringList labels )
{
	ui->chainList->clear();
	ui->chainList->addItems(labels);
}

void FdWidget::setKeyframeSlider( int N )
{
	ui->keyframeSlider->setMaximum(N-1);
}

void FdWidget::selectUnit()
{
	QList<QListWidgetItem *> selItems = ui->unitList->selectedItems();

	if (!selItems.isEmpty())
	{
		emit(unitSelectionChanged(selItems.front()->text()));
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

void FdWidget::onKeyframeSliderValueChanged()
{
	int idx = ui->keyframeSlider->value();
	int N = ui->keyframeSlider->maximum();
	double t = idx / (double)N;

	ui->keyframeTime->blockSignals(true);
	ui->keyframeTime->setValue(t);
	ui->keyframeTime->blockSignals(false);

	plugin->f_manager->selectKeyframe(idx);
}

void FdWidget::onKeyframTimeChanged()
{
	int N = ui->keyframeSlider->maximum();
	double t = ui->keyframeTime->value();
	int idx = (int)(t * N);

	ui->keyframeSlider->blockSignals(true);
	ui->keyframeSlider->setValue(idx);
	ui->keyframeSlider->blockSignals(false);

	plugin->f_manager->selectKeyframe(idx);
}
