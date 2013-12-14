#include "MainWindow.h"
#include <phonon/phonon>
#include <QRadioButton>
#include <QFileInfo>
#include <QFileDialog>
#include <QUuid>
#include <QHostInfo>
#include <QMessageBox>

#include "Screens/MyDesigner.h"

MyDesigner * designer = NULL;
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

	for(int i = 0; i < ui.screens->count(); i++)
		ui.screens->setTabEnabled(i, true);
	
	// Create custom widget
	designWidget = new Ui::DesignWidget();

	// Add custom widgets to each screen
	designWidget->setupUi(ui.designFrame);

	initDesign();
	

	// Connections
	connect(ui.screens, SIGNAL(currentChanged(int)), SLOT(screenChanged(int)));
	connect(ui.actionNewScene, SIGNAL(triggered()), SLOT(addNewScene()));
	connect(ui.actionImportObject, SIGNAL(triggered()), SLOT(importObject()));
	connect(ui.actionExportObject, SIGNAL(triggered()), SLOT(exportObject()));
	//nextButtonTutorial(); // to test Designer
	//nextButtonEvaluate(); // test Send data

	initTutorial();
}

MainWindow::~MainWindow()
{
	if (designWidget)
		delete designWidget;
	if(tutorPanel)
		delete tutorPanel;
}

void MainWindow::addNewScene()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	designer->newScene();

	QApplication::restoreOverrideCursor();
}

void MainWindow::importObject()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	
	QString fileName = QFileDialog::getOpenFileName(designer, "Import object", DEFAULT_FILE_PATH, "Mesh Files (*.obj)"); 
	if (fileName.isNull()) return;

	//designer->loadMesh(fileName);

	DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();

	QApplication::restoreOverrideCursor();
}

void MainWindow::exportObject()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QString fileName = QFileDialog::getSaveFileName(designer, "Export object", DEFAULT_FILE_PATH, "Mesh Files (*.obj)"); 
	if (fileName.isNull()) return;

	//designer->activeObject()->saveObj(fileName);

	//designer->saveConfig(fileName);
	//designer->saveLCC(filename);

	DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();
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
	//QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	VideoToolbar *vti = new VideoToolbar;
	ui.designLayout->addWidget(vti);

	// Hack: avoid dealing with Unicode here..
	//ui.checkmarkLabel->hide();

	// Add viewer
	clearLayoutItems(designWidget->viewerAreaLayout);
	designer = new MyDesigner(designWidget);
	designWidget->viewerAreaLayout->addWidget(designer);

	// Connect selection buttons
	designer->connect(designWidget->selectCuboidButton, SIGNAL(clicked()), SLOT(selectPrimitiveMode()));
	//designer->connect(designWidget->selectCurveButton, SIGNAL(clicked()), SLOT(selectCurveMode()));
	//designer->connect(designWidget->selectCameraButton, SIGNAL(clicked()), SLOT(selectCameraMode()));
	//designer->connect(designWidget->selectStackingButton, SIGNAL(clicked()), SLOT(selectStackingMode()));

	// Connect transformation tools buttons
	designer->connect(designWidget->moveButton, SIGNAL(clicked()), SLOT(moveMode()));
	designer->connect(designWidget->rotateButton, SIGNAL(clicked()), SLOT(rotateMode()));
	designer->connect(designWidget->scaleButton, SIGNAL(clicked()), SLOT(scaleMode()));
	designer->connect(designWidget->splitButton, SIGNAL(clicked()), SLOT(splitingMode()));
	
	// Connect deformers buttons
	//designer->connect(designWidget->ffdButton, SIGNAL(clicked()), SLOT(setActiveFFDDeformer()));
	//designer->connect(designWidget->voxelButton, SIGNAL(clicked()), SLOT(setActiveVoxelDeformer()));
	designer->connect(designWidget->undoButton, SIGNAL(clicked()), SLOT(Undo()));

	//int numTasks = tasksFileName.size();

	//for(int i = 0; i < numTasks; i++)
	//{
	//	QString shortName = QFileInfo(tasksFileName.at(i)).baseName();
	//	tasksLabel.push_back(new QLabel(shortName));

	//	// Mark as unfinished
	//	tasksLabel[i]->setStyleSheet(taskStyle(0));

	//	ui.tasksLayout->addWidget(tasksLabel[i], 0, i);
	//}

	//QApplication::restoreOverrideCursor();

	//loadNextTask();

	ui.screens->setTabEnabled(DESIGN_SCREEN, true);
	setScreen(DESIGN_SCREEN);
}

QWidget * MainWindow::getScreen( SCREENS screenIndex )
{
	return ui.screens->widget(screenIndex);
}

void MainWindow::setScreen( SCREENS screenIndex )
{
	ui.screens->setTabEnabled(screenIndex, true);
	ui.screens->setCurrentWidget(getScreen(screenIndex));
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

void MainWindow::screenChanged(int newScreenIndex)
{
	// Stop video in all cases
	if(v) v->stop();

	// Check if we changed to tutorial screen, if so enable video
	if(newScreenIndex == TUTORIAL_SCREEN)
	{
		if(v && v->stopped()) 
			v->start();
	}
}
