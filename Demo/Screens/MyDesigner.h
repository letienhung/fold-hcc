#pragma once

#include <QQueue>
#include <QStack>

#include "qglviewer/qglviewer.h"
#include "qglviewer/ManipulatedFrame.h"
#include "GraphManager.h"
#include "SurfaceMeshNormalsHelper.h"
#include "SurfaceMeshHelper.h"

#include "UiUtility/QManualDeformer.h"

using namespace SurfaceMesh;
using namespace qglviewer;

#include "ui_DesignWidget.h"
//#include "ui_RotationWidget.h"
//#include "ui_ScaleWidget.h"
//#include "ui_TranslationWidget.h"
//#include "Screens/FoldemLib/Graph.h"

#define EPSILON 1.0e-6

enum ViewMode { CAMERAMODE, SELECTION, MODIFY };
enum SelectMode { SELECT_NONE, MESH, VERTEX, EDGE, FACE, AABB};
enum TransformMode { NONE_MODE, TRANSLATE_MODE, ROTATE_MODE, SCALE_MODE, SPLIT_MODE};

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

	//void drawObject();
	//void drawObjectOutline();
	
	// Draw handles
	bool showGraph;
	bool showObject;
	bool isSceneEmpty;

	// Mouse & Keyboard stuff
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void wheelEvent(QWheelEvent* e);
	void keyPressEvent(QKeyEvent *e);

	// SELECTION
	SelectMode selectMode;
	QVector<int> selection;
	void setSelectMode(SelectMode toMode);	
	void postSelection(const QPoint& point);

	// Tool mode
	TransformMode transformMode;

	// Object in the scene
	GraphManager* gManager;
	BBox * mBox;
	
	GraphManager* activeObject();
	bool isEmpty();
	void setActiveObject(GraphManager* newGm);
	void newScene();

	// Deformer
	ManipulatedFrame * activeFrame;
	QManualDeformer * defCtrl;

	//Load Graph and Mesh
	void loadObject(QString fileName);

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
	//Ui::RotationWidget *rotWidget;
	//Ui::ScaleWidget *scaleWidget;
	//Ui::TranslationWidget *transWidget;

	QString viewTitle;

	//void setSelectMode( SelectMode toMode );

	// Data
	int editTime();

public slots:
    void showGraphStateChanged(int state)
	{
		this->showGraph = state;
		updateGL();
	}

	void showObjectStateChanged(int state)
	{
		this->showObject = state;
		updateGL();
	}
	
	// Select buttons
	//void selectAABBMode();
	void selectCameraMode();

	// Transform buttons
	void moveMode();
	void rotateMode();
	void scaleMode();
	void drawTool();
	//void pushAABB();

	void updateActiveObject();

	void print(QString message, long age = 1000);
	void dequeueLastMessage();

	void cameraMoved();
											
	void saveObject(QString filename);
	//void loadLCC(QString filename);
	void updateWidget();
	
private:
	// DEBUG:
	std::vector<Vec> debugPoints;
	std::vector< std::pair<Vec,Vec> > debugLines;
	std::vector< Geom::Plane > debugPlanes;
	void drawDebug();

private:
	void drawCircleFade(Vec4d &color, double radius = 1.0);
	void drawMessage(QString message, int x = 10, int y = 10, Vec4d &backcolor = Vec4d(0,0,0,0.25), Vec4d &frontcolor = Vec4d(1.0,1.0,1.0,1.0));
	void drawShadows();
	void drawViewChanger();
	double skyRadius;

	/*void transformCuboid(bool modifySelect = true);
	void transformAABB(bool modifySelect = true);
	void splitCuboid(bool modifySelect = true);*/

	void toolMode();
	void selectTool();
	void clearButtons();

signals:
	void objectInserted();
	void objectDiscarded( QString );
	void objectUpdated();

};
