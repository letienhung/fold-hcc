#pragma once

#include <QQueue>
#include <QStack>

#include "qglviewer/qglviewer.h"
#include "qglviewer/ManipulatedFrame.h"
#include "GraphManager.h"
#include "SurfaceMeshNormalsHelper.h"
#include "SurfaceMeshHelper.h"

#include "UiUtility/BBox.h"

#include "UiUtility/GL/VBO/VBO.h"
#include "CustomDrawObjects.h"

using namespace SurfaceMesh;
using namespace qglviewer;

#include "ui_DesignWidget.h"

enum ViewMode { CAMERAMODE, SELECTION, MODIFY };
enum SelectMode { SELECT_NONE, CUBOID, BOX};

class MyDesigner : public QGLViewer{
	Q_OBJECT

public:
	MyDesigner(Ui::DesignWidget * useDesignWidget, QWidget * parent = 0);
	~MyDesigner(){}

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

	//void animate();

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

	// SELECTION
	SelectMode selectMode;
	void setSelectMode(SelectMode toMode);	
	void postSelection(const QPoint& point);

	// Object in the scene
	GraphManager* gManager;
	BBox * mBox;
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
	QPoint startMousePos2D;
	QPoint currMousePos2D;
	qglviewer::Vec startMouseOrigin, currMouseOrigin;
	qglviewer::Vec startMouseDir, currMouseDir;

	// For scale tool
	SurfaceMesh::Vec3d startScalePos, currScalePos;
	Vec3d scaleDelta; // for visualization
	double loadedMeshHalfHight;

	// Hack
	Ui::DesignWidget * designWidget;

	QString viewTitle;

public slots:
	// Select buttons
	void selectCuboidMode();
	void selectCameraMode();
	void selectAABBMode();

	void updateActiveObject();

	void print(QString message, long age = 1000);
	void dequeueLastMessage();

	void cameraMoved();
	
	//Load Graph and Mesh
	void loadObject();
	void saveObject();

	// visualization
	void showCuboids(int state);
	void showGraph(int state);
	void showModel(int state);
	
private:
	// DEBUG:
	std::vector<Vec> debugPoints;
	std::vector< std::pair<Vec,Vec> > debugLines;
	std::vector< PolygonSoup > debugPlanes;
	void drawDebug();

private:
	// Draw Scene
	void drawCircleFade(Vec4d &color, double radius = 1.0);
	void drawMessage(QString message, int x = 10, int y = 10, Vec4d &backcolor = Vec4d(0,0,0,0.25), Vec4d &frontcolor = Vec4d(1.0,1.0,1.0,1.0));
	void drawShadows();
	void drawViewChanger();
	double skyRadius;

	// Set buttons for different mode
	void selectTool();
	void clearButtons();

signals:
	void objectInserted();
	void objectDiscarded();
	void objectUpdated();
};
