#include "MainWindow.h"
#include <phonon/phonon>
#include <QRadioButton>
#include <QFileInfo>
#include <QFileDialog>
#include <QUuid>
#include <QHostInfo>
#include <QMessageBox>

QString DEFAULT_FILE_PATH = "..//data";

// Theora player
#include "Screens/videoplayer/gui_player/VideoWidget.h"
#include "Screens/videoplayer/gui_player/VideoToolbar.h"
VideoWidget * v = NULL;

bool isVideo = false;

class SleeperThread : public QThread
{public:static void msleep(unsigned long msecs){QThread::msleep(msecs);}};

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	/*for(int i = 0; i < ui.screens->count(); i++)
	ui.screens->setTabEnabled(i, true);*/
	
	// Create custom widget
	designWidget = new Ui::DesignWidget();
	evalWidget = new Ui::EvaluateWidget();

	// Add custom widgets to each screen
	designWidget->setupUi(ui.designFrame);
	evalWidget->setupUi(ui.evalFrame);

	mFManager = new FoldManager();

	initDesign();
	initEvaluation();
	initQuickView();

	// Connections
	connect(ui.actionNewScene, SIGNAL(triggered()), SLOT(addNewScene()));
	connect(ui.actionImportObject, SIGNAL(triggered()), SLOT(importObject()));
	connect(ui.actionExportObject, SIGNAL(triggered()), SLOT(exportObject()));

	connect(designWidget->FoldButton, SIGNAL(clicked()),SLOT(fold()));

	//initTutorial();
}

MainWindow::~MainWindow()
{
	if (designWidget)
		delete designWidget;
	/*if (mGManager)
	delete mGManager;*/
	//if (activeViewer)
	//	delete activeViewer;
	if (viewers.size()){
		//foreach(QuickMeshViewer *v, viewers){
		//	delete v;
		//}
		viewers.clear();
	}
	//if(tutorPanel)
		//delete tutorPanel;
}

void MainWindow::addNewScene()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	designer->newScene();
	animator->newScene();
	
	for(int i = 0; i < numViewer; i++){
		if(viewers[i]->mGraph)
			viewers[i]->clearGraph();
	}

	//mFManager = NULL;
	mFManager = new FoldManager;

	QApplication::restoreOverrideCursor();
}

void MainWindow::importObject()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	designer->newScene();
	animator->newScene();

	for(int i = 0; i < numViewer; i++){
		if(viewers[i]->mGraph)
			viewers[i]->clearGraph();
	}

	//mFManager = NULL;
	mFManager = new FoldManager;

	designer->loadObject();

	//DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();

	this->setFoldManager(designer->fManager);
	animator->setActiveObject(designer->fManager);

	QApplication::restoreOverrideCursor();
}

void MainWindow::exportObject()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	designer->saveObject();

	//DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();
	QApplication::restoreOverrideCursor();
}

void MainWindow::initTutorial()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	// Load tutorial video
	if(!v)
	{
		v = new VideoWidget;
		VideoToolbar *vt = new VideoToolbar;

		isVideo = QFileInfo("data//tutorial.ogm").exists();

		if(isVideo)
		{
			tutorPanel = new TutorialPanel(v,vt);
			v->loadVideo("data/tutorial.ogm"); 
			
		}
	   // tutorPanel->setWindowFlags(Qt::WindowStaysOnTopHint);//Qt::FramelessWindowHint|
		tutorPanel->raise();
		tutorPanel->activateWindow();
		tutorPanel->show();
	}

	QApplication::setActiveWindow(tutorPanel); 
	QApplication::restoreOverrideCursor();
	//QCoreApplication::processEvents();
}

void MainWindow::initDesign()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	//VideoToolbar *vti = new VideoToolbar;
	//ui.designFrame->addWidget(vti);

	// Add viewer
	clearLayoutItems(designWidget->viewerAreaLayout);
	designer = new MyDesigner(designWidget);
	designWidget->viewerAreaLayout->addWidget(designer);

	/*mGManager = designer->gManager;*/

	// Connect selection buttons
	designer->connect(designWidget->selectCuboidButton, SIGNAL(clicked()), SLOT(selectCuboidMode()));
	designer->connect(designWidget->selectCameraButton, SIGNAL(clicked()), SLOT(selectCameraMode()));
	designer->connect(designWidget->pushButton, SIGNAL(clicked()), SLOT(selectAABBMode()));

	QApplication::restoreOverrideCursor();
}

void MainWindow::initEvaluation()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	// Add viewer
	clearLayoutItems(evalWidget->viewerAreaLayout);
	animator = new MyAnimator(evalWidget);
	evalWidget->viewerAreaLayout->addWidget(animator );
	animator->addSlider();
	animator->setActiveObject(designer->fManager);

	QApplication::restoreOverrideCursor();
}

void MainWindow::initQuickView()
{
	numViewer = 8;
	selectedId = 0;
	index = 0;
	
	if(viewers.size())
	   viewers.clear();
	
	viewers.resize(numViewer);
	for(int i = 0; i < numViewer; i++){
		//if(viewers[i]->mGraph)
			//viewers[i]->clearGraph();
		viewers[i] = new QuickMeshViewer;
		connect(viewers[i], SIGNAL(gotFocus(QuickMeshViewer*)), SLOT(setActiveViewer(QuickMeshViewer*)));
	}

	numActiveViewers = 0;
	activeViewer = viewers[0];

	for(int i = 0; i < numViewer; i++)
		ui.ThumbGrid->addWidget(viewers[i]);

	connect(ui.horizontalScrollBar, SIGNAL(valueChanged(int)), SLOT(loadGraphs()));
	//connect(mFManager, SIGNAL(resultsGenerated(int)), SLOT(reloadGraphs(int)));
	connect(designer, SIGNAL(resultsGenerated()), SLOT(reloadGraphs()));
	connect(this, SIGNAL(selectResult(int)), animator, SLOT(loadResult(int)));
}

void MainWindow::clearLayoutItems(QLayout * layout)
{
	if ( layout != NULL )
	{
		QLayoutItem* item;
		while ( ( item = layout->takeAt( 0 ) ) != NULL )
		{
			delete item->widget();
			delete item;
		}
		//delete w->layout();
	}
}

void MainWindow::showEvent( QShowEvent * event )
{
	activeViewer->setFocus();
}

void MainWindow::showNumViewers( int n )
{
	int count = 0, activeCount = 0;

	for(int i = 0; i < numViewer; i++){
		if(count++ < n)
		{
			viewers[i]->isActive = true;
			viewers[i]->clearGraph();
			viewers[i]->resetView();

			activeCount++;
		}
		else
			viewers[i]->isActive = false;
	}

	refresh();

	numActiveViewers = activeCount;
}

void MainWindow::setFoldManager(FoldManager *fm)
{
	mFManager = fm;
}

void MainWindow::reloadGraphs()
{
	int n = mFManager->results.size();
	int numPages = ceil(double(n) / double(numViewer)) - 1;

	ui.horizontalScrollBar->setRange(0, numPages);
	ui.horizontalScrollBar->setValue(0);

	if(n)
		loadGraphs();
	else
	{
		showNumViewers(0);
		refresh();
	}
}

void MainWindow::loadGraphs(QString using_path)
{
	path = using_path;
	
	// Get list of files
	QStringList filters;
	filters << "*.xml";
	files = QDir(path).entryList(filters);

	int numPages = ceil(double(files.size()) / double(numViewer)) - 1;

	ui.horizontalScrollBar->setRange(0, numPages);
	ui.horizontalScrollBar->setValue(0);

	if(files.size())
		loadGraphs();
	else
	{
		showNumViewers(0);
		refresh();
	}
}

void MainWindow::loadGraphs()
{
	loadCurrentGraphs();
}

void MainWindow::refresh()
{
	for(int i = 0; i < numViewer; i++)
		viewers[i]->updateGL();
}

void MainWindow::loadCurrentGraphs()
{
	int n = mFManager->results.size();
	index = ui.horizontalScrollBar->value() * numViewer;
	int curActive = Min(numViewer, n - index);

	showNumViewers(curActive);

	int c = 0;

	for(int i = 0; i < numViewer; i++){
		viewers[i]->clearGraph();
		viewers[i]->resetView();		
	}

	for(int i = 0; i < numViewer; i++){

		if(index + c > n - 1) return;

		//QString fileName = path + "\\" + files[index + c];
		int size = mFManager->results[index+c].size();
		FdGraph *currFrame = mFManager->results[index+c][size - 1]->deepClone();
		new LoaderThread(viewers[i], currFrame);
		viewers[i]->mIdx = index+c;
		c++; 
		if(c > curActive) return;
	}
}

void MainWindow::setActiveViewer( QuickMeshViewer* v)
{
	activeViewer = v;
	selectedId = activeViewer->mIdx;
	emit(selectResult(selectedId));
}

QString MainWindow::selectedFile()
{
	if(activeViewer) return activeViewer->graphFileName();
	return "";
}

void MainWindow::fold()
{

}

LoaderThread::LoaderThread(QuickMeshViewer * v, QString file_name)
{
	this->viewer = v;
	this->fileName = file_name;
	this->start();
}

LoaderThread::LoaderThread(QuickMeshViewer * v, FdGraph *fdGr)
{
	this->viewer = v;
	this->mGraph = fdGr;
	this->start();
}

void LoaderThread::run()
{
	if(mGraph)
		this->viewer->setGraph(mGraph);
	else if(fileName != "")
	   this->viewer->loadGraph(fileName);
	QThread::exit();
}
