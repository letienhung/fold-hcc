#pragma once

#include "interfaces/ModePluginDockWidget.h"
#include "SurfaceMeshPlugins.h"	
#include "SurfaceMeshPlugins.h"
#include "SurfaceMeshHelper.h"
#include "StarlabDrawArea.h"

#include "Graph.h"
#include "DeformHandle.h"
#include "Deformer.h"
//#include "FuiWidget.h"

#include <QVector>
#include <QList>
#include <QPair>

#include "TransformationPanel.h"

class FuiWidget;

class FoldabilizerUi : public SurfaceMeshModePlugin
{
	Q_OBJECT
	Q_INTERFACES(ModePlugin)

	// Plugin interfaces
	QIcon icon(){ return QIcon("image/icon.png"); }
	void create();
	void destroy();
	void decorate();

	//Draw handle
	void drawHandle();

	void resetScene();

public:
    FoldabilizerUi();
	~FoldabilizerUi();

	Graph *mHCCGraph;
	SurfaceMeshModel *mMesh;

	TranslationPanel *transPanel;
	ScalePanel *scalePanel;
	RotationPanel *rotPanel;

public:
	// User interactions
	bool wheelEvent(QWheelEvent *);
	bool mouseMoveEvent(QMouseEvent*);
	bool keyPressEvent(QKeyEvent *);
	//bool endSelection(const QPoint& p);

	void setSelectMode(DEFORM_MODE toMode);

	//Draw a proposed contacting type of the focused node with another node
	//Return the offset to align node N with another
	//Vector3 proposeContact(Node *focusN);
	////Automatically align box if the proposed contact is accepted by users
	//void alignBoxes(Node *focusN, Vector3 &offset);

private:
	FuiWidget *widget;

	//Deformation Mode (Disabled before triggered)
	DEFORM_MODE mCurMode;
	DeformHandle *deformHandle;
	//ManipulatedFrame *deformHandle;
	Deformer *deformer;

	// Factor for deformation
	//Vector3 mVec;

	// Mouse cursor position
	QPoint cursorPos;

private:
	//align Boxes
	//void alignBoxPair(Node *n1, Node *n2);
	
	//Propose contacting type of the focused node with another node
	//Return list of parallel axis pair index
	/*QList<QPair<int, int>> alignAxisPair(Node *n1, Node *n2);
	float calDistance(Node *n1, Node *n2, Point &p1, Point &p2);
	bool isContained(EdgeType &edge, Point &p);
	Node* proposeNeighborNode(Node *focusN, Point &p1, Point &p2);
	Line* proposeLineContact(Node *focusN, Node *neighborN, QList<QPair<int, int>> &pairs, Point &p1, Point &p2, Vector3 &offset);
	Plane* proposePlaneContact(Node *focusN, Node *neighborN, QList<QPair<int, int>> &pairs, Point &p1, Point &p2, Vector3 &offset);
	QPair<EdgeType, EdgeType> findClosestLines(Node *focusN, Node *neighborN,Point &p1, Point &p2);
	QPair<FaceType, FaceType> findClosestFaces(Node *focusN, Node *neighborN,Point &p1, Point &p2);
	BoxContactRegion* proposeRegion(Node *focusN, Node *neighborN, Point &p1, Point &p2, Vector3 &offset);
	ContactType proposeContactType(Node *focusN, Node *neighborN);*/
	
	//Deform HCC node
	Node* getFocusNode();

	//Initialize deformer
	void initDeform();

	//update HCC graph after create box
	void updateHCC();
	void setMutEx(Node *focusedNode);

	//Draw handles for different modes
	void drawTranslate(double scaling);
	void drawRotate(double scaling);
	void drawScale(double scaling);

public slots:
	//LCC maker
	void loadLCC();
	void saveLCC();
	void showLCC(int state);
	//void showModel();
	//void fitBox();
	void createBox();
	//void alignBox();

	void Deform();

	////Foldabilizer
	//void foldabilize();
	//void loadConfig();
	//void saveConfig();
};


