#pragma once

#include <QQueue>
#include <QStack>

#include "qglviewer/qglviewer.h"
#include "qglviewer/ManipulatedFrame.h"
#include "GraphManager.h"
#include "SurfaceMeshNormalsHelper.h"
#include "SurfaceMeshHelper.h"

#include "UiUtility/BBox.h"
#include "Screens/videoplayer/gui_player/VideoToolbar.h"

#include "UiUtility/GL/VBO/VBO.h"
#include "CustomDrawObjects.h"

using namespace SurfaceMesh;
using namespace qglviewer;

#include "ui_EvaluateWidget.h"

class MyAnimator : public QGLViewer{
	Q_OBJECT

public:
	MyAnimator(Ui::EvaluateWidget * useAnimWidget, QWidget * parent = 0);
	~MyAnimator(){}

	//Draw 
	void init();
	void setupCamera();
	void setupLights();
	void preDraw();
	void draw();
	void drawWithNames();
	void postDraw();
	void resetView();
	void beginUnderMesh();
	void endUnderMesh();
	void drawOSD();

	// VBOS
	QMap<QString, VBO> vboCollection;
	void updateVBOs();
	QVector<uint> fillTrianglesList(FdNode* n);
	void drawObject();
	void drawObjectOutline();

	// Mouse & Keyboard stuff
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void wheelEvent(QWheelEvent* e);
	void keyPressEvent(QKeyEvent *e);

	// Object in the scene
	GraphManager* gManager;
	bool isShow;

	GraphManager* activeManager();
	FdGraph* activeScaffold();
	bool isEmpty();
	void setActiveObject(GraphManager* newGm);
	void newScene();

	// Deformer
	ManipulatedFrame * activeFrame;

	// TEXT ON SCREEN
	QQueue<QString> osdMessages;
	QTimer *timerScreenText;

	// Mouse state
	bool isMousePressed;

	// For scale
	double loadedMeshHalfHight;

	VideoToolbar *vti;

	// Hack
	Ui::EvaluateWidget * evalWidget;

	QString viewTitle;

	public slots:
		void updateActiveObject();

		void print(QString message, long age = 1000);
		void dequeueLastMessage();

		void cameraMoved();

		//Load Configuration
		void loadConfig(int configId);
		//void saveObject();

		//Animation
		void startAnimation();
		void stopAnimation();
		void animate();
		void toggleplay(int frameId);

public:
	// DEBUG:
	std::vector<Vec> debugPoints;
	std::vector< std::pair<Vec,Vec> > debugLines;
	std::vector< PolygonSoup > debugPlanes;
	void drawDebug();
	QVector<FdGraph *> mGraphs;

private:
	// Draw Scene
	void drawCircleFade(Vec4d &color, double radius = 1.0);
	void drawMessage(QString message, int x = 10, int y = 10, Vec4d &backcolor = Vec4d(0,0,0,0.25), Vec4d &frontcolor = Vec4d(1.0,1.0,1.0,1.0));
	void drawShadows();
	void drawViewChanger();
	double skyRadius;

signals:
	void objectInserted();
	void objectDiscarded();
	void objectUpdated();
	void setSliderValue(int);
};
