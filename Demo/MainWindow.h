#pragma once

#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"

enum SCREENS{ TUTORIAL_SCREEN, DESIGN_SCREEN, EVALUATE_SCREEN };

// Theora player
#include "Screens/videoplayer/gui_player/VideoWidget.h"
#include "Screens/videoplayer/gui_player/VideoToolbar.h"

#include "ui_DesignWidget.h"
#include "ui_TutorialWidget.h"

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

	Ui::DesignWidget * designWidget;
	TutorialPanel  *tutorPanel;

	void initTutorial();
	void initDesign();

public slots:
	QWidget * getScreen(SCREENS screenIndex);
	void setScreen( SCREENS screenIndex );

	void addNewScene();
	void importObject();
	void exportObject();

	void clearLayoutItems(QLayout * layout);

	void screenChanged(int newScreenIndex);

private:
	Ui::MainWindow ui;

	//QStringList tasksFileName, tasksFiles;
	//QMap<QString, double> tasksTarget;
	//QVector<QLabel *> tasksLabel;
	//QMap<QString, QMap<QString, QString> > taskResults;
};

