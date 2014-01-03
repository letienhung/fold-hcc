#pragma once

#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"
#include "Screens/UiUtility/QuickMeshViewer.h"
#include "Screens/MyDesigner.h"
#include "Screens/MyAnimator.h"
#include "FoldManager.h"

// Theora player
#include "Screens/videoplayer/gui_player/VideoWidget.h"
#include "Screens/videoplayer/gui_player/VideoToolbar.h"

#include "ui_DesignWidget.h"
#include "ui_TutorialWidget.h"
#include "ui_EvaluateWidget.h"

class TutorialPanel : public QWidget
{
	Q_OBJECT

public:
	TutorialPanel(VideoWidget *v, VideoToolbar *vt)
	{
	   tutorWidget.setupUi(this);
	   mv = v;
	   mvt =vt;

	   // Connect
	   connect(mvt->ui->playButton, SIGNAL(clicked()), v, SLOT(togglePlay()));
	   connect(mvt, SIGNAL(valueChanged(int)), mv, SLOT(seekVideo(int)));
	   connect(mv, SIGNAL(setSliderValue(int)), mvt->ui->slider, SLOT(setValue(int)));

	   tutorWidget.centralLayout->addWidget(mv);
	   tutorWidget.centralLayout->addWidget(mvt);

	}

	~TutorialPanel()
	{
		if(mv)
			delete mv;
		if(mvt)
			delete mvt;
	}

	VideoWidget *mv;
	VideoToolbar *mvt;

	Ui::TutorialWidget tutorWidget;
};

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

	//Ui component
	Ui::DesignWidget * designWidget;
	TutorialPanel  *tutorPanel;
	Ui::EvaluateWidget * evalWidget;

	MyDesigner  *designer;
	MyAnimator *animator;
	//GraphManager* mGManager;
	FoldManager* mFManager;

	//Quick Mesh Views to visualize multiple results
	QVector<QuickMeshViewer*> viewers;
	QuickMeshViewer* activeViewer;
	int numViewer;
	int numActiveViewers;
	// The index of selected result to be visualized in animation
	int selectedId;
	// Curent base index of slider
	int index;
	//File name of selected configuration
	QString path;
	//List of result 
	QStringList files;
	QString selectedFile();
	
	//Init Ui component
	void initTutorial();
	void initDesign();
	void initEvaluation();
	void initQuickView();

protected:
	virtual void showEvent(QShowEvent * event);

public slots:
	//Slots for tool bar component
	void addNewScene();
	void importObject();
	void exportObject();
    
	//Reset layout
	void clearLayoutItems(QLayout * layout);
	
	//Slots for Quick Mesh Viewer
	// Slot to be connected to the signal resultsGenerated in FoldManager
	void reloadGraphs();
	void loadGraphs(QString using_path);
	void loadGraphs();
	void showNumViewers(int n);
	void loadCurrentGraphs();
	void setActiveViewer(QuickMeshViewer*);
	void refresh();

	void fold();

private:
	Ui::MainWindow ui;

	void setFoldManager(FoldManager *fm);

signals:
	void selectResult(int);

};

#include <QThread>
class LoaderThread : public QThread{
	Q_OBJECT
public:
	LoaderThread(QuickMeshViewer *, QString);
	LoaderThread(QuickMeshViewer *, FdGraph *);
	QuickMeshViewer * viewer;
	QString fileName;
	FdGraph *mGraph;
protected:
	void run();
};

