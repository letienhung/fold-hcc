#include "FoldabilizerUi.h"
#include "FuiWidget.h"

#include <QFileDialog>
#include <QDebug>

#include "Numeric.h"

#include "qglviewer/qglviewer.h"

QString DEFAULT_FILE_PATH = "..\\..\\data";

FoldabilizerUi::FoldabilizerUi()
{
	widget = NULL;
	mHCCGraph = new Graph();
	mMesh = NULL;//mesh();
	mCurMode = SELECT;
	
	this->deformer = NULL;
	this->deformHandle = NULL;

	transPanel = new TranslationPanel();
	scalePanel = new ScalePanel();
	rotPanel = new RotationPanel();

	scalePanel->hide();
	rotPanel->hide();
	transPanel->hide();
}

FoldabilizerUi::~FoldabilizerUi()
{
	if(widget)
		delete widget;
	if(mHCCGraph)
		delete mHCCGraph;
	if(deformer)
		delete deformer;
	if(deformHandle)
		delete deformHandle;
}

void FoldabilizerUi::create()
{
	if (!widget)
	{
		widget = new FuiWidget(this);

		ModePluginDockWidget *dockwidget = new ModePluginDockWidget("Foldabilizer UI", mainWindow());
		dockwidget->setWidget(widget);
		mainWindow()->addDockWidget(Qt::RightDockWidgetArea, dockwidget);
	}
}

void FoldabilizerUi::setSelectMode(DEFORM_MODE toMode)
{
	mCurMode = toMode;
}

bool FoldabilizerUi::wheelEvent(QWheelEvent * e)
{
	cursorPos = e->pos();
	if(e->modifiers() == Qt::NoButton) return false;

	drawArea()->updateGL();

	return false;
}

bool FoldabilizerUi::mouseMoveEvent(QMouseEvent * e)
{
	//QGLViewer::mouseMoveEvent(e);
	cursorPos = e->pos();
	if(e->modifiers() == Qt::NoButton) //&& mCurMode != SELECT) 
		return false;
	if ((e->buttons() & Qt::LeftButton) && (e->modifiers() == Qt::ControlModifier) && mCurMode != SELECT)
	   drawArea()->select(e->pos());
		
	if ((e->buttons() & Qt::RightButton) && (e->modifiers() == Qt::ShiftModifier))
	{
		QMenu menu( drawArea());

		QAction* SelectAction = menu.addAction("Select Cuboid..");
		QAction* TranslateAction = menu.addAction("Translate..");
		QAction* RotateAction = menu.addAction("Rotate..");
		QAction* ScaleAction = menu.addAction("Scale..");
		//menu.addSeparator();

		QAction* action = menu.exec(e->globalPos()); // show menu

		// Selections
		if(action == SelectAction){
			setSelectMode(SELECT);
			disconnect(deformHandle, SIGNAL(manipulated()), this, SLOT(Deform()));
			scalePanel->hide();
			rotPanel->hide();
			transPanel->hide();	
		}
		else{
			if(action == TranslateAction)
				setSelectMode(TRANSMODE);

			else if(action == RotateAction)
				setSelectMode(ROTATEMODE);	
	
			else if(action == ScaleAction)
				setSelectMode(SCALEMODE);

			initDeform();
		}
	}
	drawArea()->updateGL();
	return true;
}

bool FoldabilizerUi::keyPressEvent(QKeyEvent *)
{
	return false;
}

//bool FoldabilizerUi::endSelection(const QPoint &p)
//{
//    return false;
//}

void FoldabilizerUi::destroy()
{

}

void FoldabilizerUi::decorate()
{
	if (mHCCGraph)
		mHCCGraph->draw();
	if (mCurMode != SELECT)
		drawHandle();
}

void FoldabilizerUi::drawHandle()
{
	if(!deformHandle) return;

	int pixels = 32;
	double scaling = 2 * pixels * drawArea()->camera()->pixelGLRatio(qglviewer::Vec(0,0,0));

	glPushMatrix();
	glMultMatrixd(deformHandle->matrix());

	glDisable(GL_LIGHTING);
	glLineWidth(3.0f);

	QString info = "Ctrl + LeftClick to select the handle.\n";
	switch (mCurMode){
		case TRANSMODE: {
			drawTranslate(scaling); 
			info += "Right click to translate.";			
		}break; 
		case SCALEMODE:	{
			drawScale(scaling); 
			info += "Right click to scale.";			
		}break;
		case ROTATEMODE: {
			drawRotate(scaling); 
			info += "Left click to rotate.";
		}break;
	}
	//Todo
	glEnable(GL_LIGHTING);

	glPopMatrix();

	glColor4d(1,1,1,1);
	drawArea()->renderText(50,50,info);
}

void FoldabilizerUi::drawTranslate(double scaling)
{
	glBegin(GL_LINES);

	glColor3d(1,0,0);
	glVertex3fv(Vec(0,0,0));
	glVertex3fv(Vec(1,0,0) * scaling);

	glColor3d(0,1,0);
	glVertex3fv(Vec(0,0,0));
	glVertex3fv(Vec(0,1,0) * scaling);

	glColor3d(0,0,1);
	glVertex3fv(Vec(0,0,0));
	glVertex3fv(Vec(0,0,1) * scaling);

	glEnd();
}
void FoldabilizerUi::drawRotate(double scaling)
{
	glBegin(GL_LINE_LOOP);
	glColor3d(1,0,0);
	for(int i =0; i < 360; i++)
	{
		float angle = 2 * PI * i / 360;
		float x = cos(angle) * scaling;
		float y = sin(angle) * scaling;
		glVertex3fv(Vec(x,y,0));
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	glColor3d(0,1,0);
	for(int i =0; i < 360; i++)
	{
		float angle = 2 * PI * i / 360;
		float y = cos(angle) * scaling;
		float z = sin(angle) * scaling;
		glVertex3fv(Vec(0,y,z));
	}
	glEnd();

	glBegin(GL_LINE_LOOP);
	glColor3d(0,0,1);
	for(int i =0; i < 360; i++)
	{
		float angle = 2 * PI * i / 360;
		float x = cos(angle) * scaling;
		float z = sin(angle) * scaling;
		glVertex3fv(Vec(x,0,z));
	}
	glEnd();
}
void FoldabilizerUi::drawScale(double scaling)
{
	glBegin(GL_LINES);

	glColor3d(1,0,0);
	glVertex3fv(Vec(0,0,0));
	glVertex3fv(Vec(1,0,0) * scaling);

	glColor3d(0,1,0);
	glVertex3fv(Vec(0,0,0));
	glVertex3fv(Vec(0,1,0) * scaling);

	glColor3d(0,0,1);
	glVertex3fv(Vec(0,0,0));
	glVertex3fv(Vec(0,0,1) * scaling);

	glEnd();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_TRIANGLES);

	glColor3d(0.75,0.75,0);
	glVertex3fv(Vec(0,0,0));
	glVertex3fv(Vec(1,0,0) * scaling/2);
	glVertex3fv(Vec(0,1,0) * scaling/2);

	glColor3d(0.75,0.75,0);
	glVertex3fv(Vec(1,0,0) * scaling/2);
	glVertex3fv(Vec(0,1,0) * scaling/2);
	glVertex3fv(Vec(0,0,1) * scaling/2);

	glColor3d(0.75,0.75,0);
	glVertex3fv(Vec(0,0,0));
	glVertex3fv(Vec(0,1,0) * scaling/2);
	glVertex3fv(Vec(0,0,1) * scaling/2);

	glColor3d(0.75,0.75,0);
	glVertex3fv(Vec(1,0,0) * scaling/2);
	glVertex3fv(Vec(0,0,0));
	glVertex3fv(Vec(0,0,1) * scaling/2);

	glEnd();
}


void FoldabilizerUi::resetScene()
{
	if (mHCCGraph && mHCCGraph->isDraw){
		drawArea()->setSceneBoundingBox(qglviewer::Vec(mHCCGraph->bbmin), qglviewer::Vec(mHCCGraph->bbmax));
	}

	drawArea()->camera()->showEntireScene();
	drawArea()->updateGL();
}

void FoldabilizerUi::initDeform()
{
	if(deformer) 
		delete deformer;
	if(deformHandle) 
		delete deformHandle;

	Node *n = getFocusNode();
	if(!n) return;
	
	deformer = new Deformer(n);
	//deformer->updateFactor(mVec);

	 // setup deformation handle
	deformHandle = new DeformHandle(Vector3(-1.5,1.5,0));
	//deformHandle = new ManipulatedFrame();
	drawArea()->setManipulatedFrame( deformHandle );
	this->connect(deformHandle, SIGNAL(manipulated()), this, SLOT(Deform()));

	switch (mCurMode){
	case TRANSMODE: {
		transPanel->updateNode(n);
		scalePanel->hide();
		rotPanel->hide();
		transPanel->show();		
					}break; 
	case SCALEMODE:	{
		scalePanel->updateNode(n);
		scalePanel->show();
		rotPanel->hide();
		transPanel->hide();				
					}break;
	case ROTATEMODE: {
		rotPanel->updateNode(n);
		scalePanel->hide();
		rotPanel->show();
		transPanel->hide();		
					 }break;
	}

	drawArea()->updateGL();
}

Node* FoldabilizerUi::getFocusNode()
{
	foreach(Node* n, mHCCGraph->nodes){
		if(n->isFocused)
			return n;
	}
	return NULL;
}

void FoldabilizerUi::updateHCC()
{
	//Todo
}

void FoldabilizerUi::setMutEx(Node *focusedNode)
{
	foreach(Node *n, mHCCGraph->nodes)
	{
	   n->isFocused = false;
	}
	focusedNode->isFocused = true;
}

void FoldabilizerUi::loadLCC()
{
	QString fileName = QFileDialog::getOpenFileName(this->widget, "Import LCC", DEFAULT_FILE_PATH, "Mesh Files (*.lcc)"); 
	if (fileName.isNull()) return;

	if (mHCCGraph->loadHCC(fileName))
		resetScene();

	DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();
}

void FoldabilizerUi::saveLCC()
{
	QString fileName = QFileDialog::getSaveFileName(this->widget, "Export LCC", DEFAULT_FILE_PATH, "Mesh Files (*.lcc)"); 
	if (fileName.isNull()) return;

	mHCCGraph->saveHCC(fileName);

	DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();
}

void FoldabilizerUi::showLCC(int state)
{
	mHCCGraph->isDraw = state;
	resetScene();
}

void FoldabilizerUi::createBox()
{
	int nsize = mHCCGraph->nbNodes();
	QVector<Vector3> xyz = XYZ();
	Node *n = new Node(Geom::Box(Point(0,0,0), xyz, Vector3(1,1,1)),QString::number(nsize, 10));

	mHCCGraph->nodes.push_back(n);
    setMutEx(n);
	mHCCGraph->computeAabb();
	resetScene();
}

void FoldabilizerUi::Deform()
{
	Node *n = getFocusNode();
	if(!n)
		return;
	deformer->updateNode(n);
	
	if(mCurMode == SCALEMODE || mCurMode == TRANSMODE){
	   Vector3 Vec = deformHandle->transform(n->mBox.Extent);
	   deformer->updateFactor(Vec);
	   
	}
	else if(mCurMode == ROTATEMODE){
	   //Eigen::Matrix3f m = deformHandle->rotate();
	   Quaternion q = deformHandle->rotate();
	   //deformer->updateRotMat(m);
	   deformer->updateRotQuan(q);
	}
	else 
		return;

	deformer->Deform(mCurMode);
	//proposeContact(n);
	updateHCC();
	deformHandle->updateStart(Vector3(-1.5,1.5,0));

	drawArea()->updateGL();			 
}

//void FoldabilizerUi::alignBox()
//{
//	foreach(Link *l, mHCCGraph->links)
//	{
//		alignBoxPair(l->node1, l->node2);
//	}
//	//Todo
//}
//void FoldabilizerUi::alignBoxPair(Node *n1, Node *n2)
//{
//	IntersectBoxBox interTest;
//	if(interTest.test(n1->mBox, n2->mBox)){
//		//Todo
//	}
//}
//
////Box contacting 
//Vector3 FoldabilizerUi::proposeContact(Node *focusN)
//{
//	if(!focusN)
//		return;
//
//	Point p1, p2;
//	Node *neighborN	= proposeNeighborNode(focusN,p1,p2);
//	if(!neighborN)
//		return;
//
//	Vector3 offset;
//	BoxContactRegion *cRegion = proposeRegion(focusN, neighborN,p1,p2, offset);
//	if (!cRegion)
//		return;
//
//	cRegion->draw(drawArea());
//	drawArea()->updateGL();	
//	return offset;
//}
//
//Node* FoldabilizerUi::proposeNeighborNode(Node *focusN, Point &p1, Point &p2)
//{
//	float curDist, minDist = 10000000;
//	Node* neigborN = NULL;
//	Point _p1, _p2;
//	foreach(Node *n, mHCCGraph->nodes){
//		if(n != focusN){
//			curDist = calDistance(focusN,n,_p1,_p2); 
//			if( curDist <= minDist){
//				neigborN = n;
//				minDist = curDist;
//				p1 = _p1;
//				p2 = _p2;
//			}   
//		}
//	}
//	return neigborN;
//}
//
//QList<QPair<int, int>> FoldabilizerUi::alignAxisPair(Node *n1, Node *n2)
//{
//	QList<QPair<int, int>> pairs;
//	for(int i = 0; i < 3; i++)
//		for(int j = 0; j < 3; j++){
//			if(n1->mBox.Axis[i] == n2->mBox.Axis[j] || n1->mBox.Axis[i] == -(n2->mBox.Axis[j]))
//				pairs.append(qMakePair(i,j));
//		}
//    return pairs;
//}
//
//ContactType FoldabilizerUi::proposeContactType(Node *focusN, Node *neighborN)
//{
//	QList<QPair<int, int>> pairs = alignAxisPair(focusN, neighborN);
//	if(pairs.size()== 0)
//		return UnAligned;
//	else if(pairs.size()==1){
//		return;
//	}
//	else{
//		return; 
//	}
//}
//
////Propose contact region
//BoxContactRegion* FoldabilizerUi::proposeRegion(Node *focusN, Node *neighborN, Point &p1, Point &p2, Vector3 &offset)
//{
//	//BoxContactRegion *r ;
//	//ContactType ct = proposeContactType(focusN, neighborN);
//	QList<QPair<int, int>> pairs = alignAxisPair(focusN, neighborN);
//	if(pairs.size()==1)
//	    return proposeLineContact(focusN, neighborN, pairs, p1, p2, offset);
//	else if(pairs.size()==3){
//		/*if(1)
//		return proposeLineContact(focusN, neighborN, pairs, p1, p2, offset);
//		else*/
//		return proposePlaneContact(focusN, neighborN, pairs, p1, p2, offset);
//	}
//	else
//	    return NULL;
//}
//
//bool FoldabilizerUi::isContained(EdgeType &edge, Point &p)
//{
//	Vector3 v1((p.x()-edge.first.x()),(p.y()-edge.first.y()),(p.z()-edge.first.z()));
//	v1 = v1/v1.norm();
//	Vector3 v2((edge.second.x()-p.x()),(edge.second.y()-p.y()),(edge.second.z()-p.z()));
//	v2 = v2/v2.norm();
//	if(v1 == v2)
//		return true;
//	else
//		return false;
//}
//
//Line* FoldabilizerUi::proposeLineContact(Node *focusN, Node *neighborN, QList<QPair<int, int>> &pairs, Point &p1, Point &p2, Vector3 &offset)
//{
//	//QMapIterator<int, int> i(map);
//	if(pairs.size() == 1){
//		Vector3 axis1 = focusN->mBox.Axis[pairs[0].first];
//		Vector3 axis2 = focusN->mBox.Axis[pairs[0].second];
//		Point p1_,p2_;
//		EdgeType edge1 = focusN->getEdgeFixPointVector(p1, axis1, p1_);//p1_ is another end
//		EdgeType edge2 = neighborN->getEdgeFixPointVector(p2, axis2, p2_);//p2_ is another end
//
//		Vector3 d(p1.x() - p2.x(),p1.y() - p2.y(),p1.z() - p2.z());
//		Vector3 d1 = d.norm() * axis2 + p2;		
//		Point p_1(d1.x(),d1.y(),d1.z()); //p1's projection on edge2
//		offset = Vector3(p_1.x()- p1.x(), p_1.y()- p1.y(), p_1.z()- p1.z());  //From edge1(p1,p1_) to edge2(p2, p2_)
//		Vector3 d2 = Vector3(p2.x(), p2.y(), p2.z())-offset;
//		Point p_2(d2.x(),d2.y(),d2.z()); //p2's projection on edge1 
//		Vector3 d3 = Vector3(p1_.x(), p1_.y(), p1_.z()) + offset;
//		Point p_1_(d3.x(),d3.y(),d3.z()); //p1_'s projection on edge2 
//		Vector3 d4 = Vector3(p2_.x(), p2_.y(), p2_.z()) - offset;
//		Point p_2_(d4.x(),d4.y(),d4.z()); //p2_'s projection on edge1 
//
//		Line* l;
//		if(isContained(edge1, p_2)&&isContained(edge1, p_2_))
//			l = new Line(p_2, p_2_, 3.0, Qt::yellow);
//		else if(isContained(edge1, p_2)&&!isContained(edge1, p_2_)){
//			if(isContained(edge2, p_1)&&!isContained(edge2, p_1_))
//			    l = new Line(p_2, p1, 3.0, Qt::yellow);
//			else if(!isContained(edge2, p_1)&&isContained(edge2, p_1_))
//			    l = new Line(p_2, p_1_, 3.0, Qt::yellow);
//			else
//				l = NULL;
//		}
//		else{
//			if(isContained(edge2, p_1)&&isContained(edge2, p_1_))
//				l = new Line(p_1, p_1_, 3.0, Qt::yellow);
//			else
//				l = NULL;
//		}
//		return l;
//	}
//	//else if(pairs.size() == 3){
//	//	//TODO
//	//	Line * l = new Line();
//	//	return l;
//	//}
//	else 
//		return NULL;
//}
//
//Plane* FoldabilizerUi::proposePlaneContact(Node *focusN, Node *neighborN, QList<QPair<int, int>> &pairs, Point &p1, Point &p2, Vector3 &offset)
//{
//	Plane* p;
//
//	QVector<QVector<Point>> fn_faces = focusN->getBoxFaces();
//	QVector<Point> fn_vertices = focusN->getBoxConners();
//
//	QVector<QVector<Point>> nb_faces = neighborN->getBoxFaces();
//	QVector<Point> nb_vertices = neighborN->getBoxConners();
//
//	return p;
//}
//
//float FoldabilizerUi::calDistance(Node *n1, Node *n2, Point &p1, Point &p2)
//{
//	QVector<Point> fn_vertices = n1->getBoxConners();
//	QVector<Point> nb_vertices = n2->getBoxConners();
//
//	float dist, minDist = 10000;
//	for(int i = 0; i < 8; i++)
//		for(int j = 0; j < 8; j++){
//			dist = (fn_vertices[i] - nb_vertices[j]).norm(); 
//			if(dist < minDist){
//				minDist = dist;
//				p1 = fn_vertices[i];
//				p2 = nb_vertices[j];
//			}
//		}
//
//	return minDist;
//}
//
//QPair<EdgeType, EdgeType> FoldabilizerUi::findClosestLines(Node *focusN, Node *neighborN,Point &p1, Point &p2)
//{
//	QVector<EdgeType> fn_edges = focusN->getEdgeIncidentOnConner(p1);
//	QVector<EdgeType> nb_edges = neighborN->getEdgeIncidentOnConner(p2);
//
//	QPair<EdgeType, EdgeType> pair;
//	Vector3 fnVec, nbVec;
//	double dist, minDist = 100000;
//	for(int i = 0; i < 3; i++)
//		for(int j = 0; j < 3; j++){
//			fnVec = Vector3((fn_edges[i].first - fn_edges[i].second).x(),
//				            (fn_edges[i].first - fn_edges[i].second).y(),
//							(fn_edges[i].first - fn_edges[i].second).z());
//			fnVec.normalize();
//			nbVec = Vector3((nb_edges[j].first - nb_edges[j].second).x(),
//				            (nb_edges[j].first - nb_edges[j].second).y(),
//				            (nb_edges[j].first - nb_edges[j].second).z());
//			nbVec.normalize();
//			if(fnVec.dot(nbVec) == 1 || fnVec.dot(nbVec) == -1){
//			   Point fnmid = (fn_edges[i].first + fn_edges[i].second)/2;
//			   Point nbmid = (nb_edges[j].first + nb_edges[j].second)/2;
//			   dist = (fnmid - nbmid).norm();
//			   if(dist < minDist){
//				   minDist = dist;
//				   pair.first = fn_edges[i];
//				   pair.second = nb_edges[j];
//			   }
//			}	   
//		}
//	return pair;
//}

Q_EXPORT_PLUGIN(FoldabilizerUi)