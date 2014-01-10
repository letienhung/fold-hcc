#include "Circle.h"
#include "AABB.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QPushButton>
#include <QMenu>
#include <QString>

#include "qglviewer/camera.h"

using namespace Geom;

QFontMetrics * fm;
// Misc.
#include "UiUtility/SimpleDraw.h"

#include "MyDesigner.h"

#include <QTime>
extern QTime * globalTimer;

MyDesigner::MyDesigner( Ui::DesignWidget * useDesignWidget, QWidget * parent /*= 0*/ ) : QGLViewer(parent)
{
	this->designWidget = useDesignWidget;

	selectMode = SELECT_NONE;
	skyRadius = 1.0;
	gManager = NULL;
	fManager = new FoldManager;
	currGraph = NULL;
	mBox = NULL;
	isMousePressed = false;
	loadedMeshHalfHight = 1.0;

	isShow = true;
	isLoaded = false;
	scalePercent = 0.0;

	fm = new QFontMetrics(QFont());

	connect(this, SIGNAL(objectInserted()), SLOT(updateGL()));

	activeFrame = new ManipulatedFrame();
	setManipulatedFrame(activeFrame);

	// TEXT ON SCREEN
	timerScreenText = new QTimer(this);
	connect(timerScreenText, SIGNAL(timeout()), SLOT(dequeueLastMessage()));
    connect(camera()->frame(), SIGNAL(manipulated()), SLOT(cameraMoved()));

	//Visualization
	connect(designWidget->showCuboid, SIGNAL(stateChanged(int)),  SLOT(showCuboids(int)));
	connect(designWidget->showGraph, SIGNAL(stateChanged(int)),  SLOT(showGraph(int)));
	connect(designWidget->showModel, SIGNAL(stateChanged(int)), SLOT(showModel(int)));

	//Set cuboid property(Splittable, Scalable)
	connect(designWidget->allowSplit, SIGNAL(stateChanged(int)),  SLOT(setSplittable(int)));
	connect(designWidget->allowScale, SIGNAL(stateChanged(int)),  SLOT(setScalable(int)));

	this->setMouseTracking(true);

	viewTitle = "View";
}

void MyDesigner::init()
{
	// Lights
	setupLights();

	// Camera
	setupCamera();

	// Material
	float mat_ambient[] = {0.1745f, 0.01175f, 0.01175f, 1.0f};
	float mat_diffuse[] = {0.65f, 0.045f, 0.045f, 1.0f};
	float mat_specular[] = {0.09f, 0.09f, 0.09f, 1.0f};
	float high_shininess = 100;

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, high_shininess);

	// Redirect keyboard
	setShortcut( HELP, Qt::CTRL+Qt::Key_H );
	setShortcut( STEREO, Qt::SHIFT+Qt::Key_S );

	camera()->frame()->setSpinningSensitivity(100.0);

	if (!VBO::isVBOSupported()) std::cout << "VBO is not supported." << std::endl;
}

void MyDesigner::setupLights()
{
	GLfloat lightColor[] = {0.9f, 0.9f, 0.9f, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
}

void MyDesigner::setupCamera()
{
	camera()->setSceneRadius(10.0);
	camera()->showEntireScene();
	camera()->setUpVector(Vec(0,0,1));
	camera()->setPosition(Vec(2,-2,2));
	camera()->lookAt(Vec());
}

void MyDesigner::cameraMoved()
{
	if(camera()->type() != Camera::PERSPECTIVE)
	{
		Vec camPos = camera()->position();
		double minAxis = Min(abs(camPos.x), Min(abs(camPos.y), abs(camPos.z)));

		if(minAxis > 0.25)
		{
			viewTitle = "View";
			camera()->setType(Camera::PERSPECTIVE);
			updateGL();
		}
	}
}

void MyDesigner::preDraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skyRadius = Max(4, Min(20, 1.5*Max(camera()->position().x, Max(camera()->position().y, camera()->position().z))));
	setSceneRadius(skyRadius);

	camera()->loadProjectionMatrix(); // GL_PROJECTION matrix
	camera()->loadModelViewMatrix(); // GL_MODELVIEW matrix

	// Anti aliasing 
	glEnable(GL_MULTISAMPLE);
	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

	// Background
	setBackgroundColor(QColor(208,212,240));

	// Draw fancy effects:
	SimpleDraw::drawSolidSphere(skyRadius,30,30, true, true); // Sky dome

	beginUnderMesh();
	double floorOpacity = 0.25; 
	if(camera()->position().z < loadedMeshHalfHight) floorOpacity = 0.05;
	drawCircleFade(Vec4d(0,0,0,floorOpacity), 4); // Floor
	
	if(designWidget->showModel->isChecked())
		drawShadows();

	endUnderMesh();

	glClear(GL_DEPTH_BUFFER_BIT);
}

void MyDesigner::drawShadows()
{
	if(!gManager || camera()->position().z < loadedMeshHalfHight) return;
	
	// Compute shadow matrix
	GLfloat floorShadow[4][4];
	GLfloat groundplane[4];
	SimpleDraw::findPlane(groundplane, Vec3d(0,0,0), Vec3d(-1,0,0), Vec3d(-1,-1,0));
	GLfloat lightpos[4] = {0.0,0.0,8,1};
	SimpleDraw::shadowMatrix(floorShadow,groundplane, lightpos);

	glScaled(1.1,1.1,0);
	glPushMatrix();
	glMultMatrixf((GLfloat *) floorShadow); /* Project the shadow. */
	glColor4d(0,0,0,0.04);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	for (QMap<QString, VBO>::iterator i = vboCollection.begin(); i != vboCollection.end(); ++i)
	{
		i->render_depth();
	}
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
	
}

void MyDesigner::draw()
{
	// No object to draw
	if (isEmpty()) return;
	if (!currGraph) //activeScaffold()
		return;

	if(selectMode == CUBOID){
		currGraph ->showCuboids(true);
	}

	// The main object
	if(designWidget->showModel->isChecked())
		drawObject();
	currGraph->draw();

	if(selectMode == BOX){
		Geom::AABB aabb = currGraph->computeAABB();
		if(mBox == NULL){
			mBox = new BBox(aabb.center(), (aabb.bbmax-aabb.bbmin)*0.5f);
			scalePercent = 0.0;
		}
		mBox->draw();
	}

	glEnable(GL_BLEND);
	//drawDebug();
}

void MyDesigner::drawDebug()
{
	glDisable(GL_LIGHTING);
	
	// DEBUG: Draw designer debug
	glBegin(GL_POINTS);
		glColor4d(1,0,0,1);
		foreach(Vec p, debugPoints) glVertex3dv(p);
	glEnd();

	glBegin(GL_LINES);
	glColor4d(0,0,1,1);
	for(std::vector< std::pair<Vec,Vec> >::iterator 
		line = debugLines.begin(); line != debugLines.end(); line++)
	{glVertex3dv(line->first); glVertex3dv(line->second);}
	glEnd();

	foreach(PolygonSoup p, debugPlanes) p.draw();

	glEnable(GL_LIGHTING);
}

void MyDesigner::drawObject()
{
	if (VBO::isVBOSupported())
	{
		updateVBOs();

		glPushAttrib (GL_POLYGON_BIT);
		glEnable (GL_CULL_FACE);

		if(isShow){
		   drawObjectOutline();
		}
		/** Draw front-facing polygons as filled */
		glPolygonMode (GL_FRONT, GL_FILL);
		glCullFace (GL_BACK);
		
		/* Draw solid object */
		for (QMap<QString, VBO>::iterator i = vboCollection.begin(); i != vboCollection.end(); ++i)
		{
			if(viewTitle != "View")
				i->render_regular(false, true);
			else
				i->render();
		}

		/* GL_POLYGON_BIT */
		glPopAttrib ();
	} 
	else{// Fall back
		//activeScaffold()->draw();
		currGraph->draw();
	}
}

void MyDesigner::drawObjectOutline()
{
	/** Draw back-facing polygons as red lines	*/
	/* Disable lighting for outlining */
	glPushAttrib (GL_LIGHTING_BIT | GL_LINE_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable (GL_LIGHTING);

	glPolygonMode(GL_BACK, GL_LINE);
	glCullFace (GL_FRONT);

	glDepthFunc (GL_LEQUAL);
	glLineWidth (Max(0.5, Min(5,skyRadius / 2.0)));

	/* Draw wire object */
	glColor3f (0.0f, 0.0f, 0.0f);
	for (QMap<QString, VBO>::iterator i = vboCollection.begin(); i != vboCollection.end(); ++i)
		i->render_depth();
	
	/* GL_LIGHTING_BIT | GL_LINE_BIT | GL_DEPTH_BUFFER_BIT */
	glPopAttrib ();
}

QVector<uint> MyDesigner::fillTrianglesList(FdNode* n)
{
	// get face indices
	QVector<uint> triangles;
	Surface_mesh::Face_iterator fit, fend = n->mMesh->faces_end();
	Surface_mesh::Vertex_around_face_circulator fvit, fvend;
	Vertex v0, v1, v2;

	for (fit = n->mMesh->faces_begin(); fit != fend; ++fit)
	{
		fvit = fvend = n->mMesh->vertices(fit);
		v0 = fvit; v1 = ++fvit; v2 = ++fvit;

		triangles.push_back(v0.idx());
		triangles.push_back(v1.idx());
		triangles.push_back(v2.idx());
	}
	return triangles;
}

void MyDesigner::updateVBOs()
{
	if(gManager && currGraph)//activeScaffold())
	{
		// Create VBO for each segment if needed
	    QVector<FdNode*> nodes = currGraph->getFdNodes();//activeScaffold()->getFdNodes();
		for (int i=0;i<nodes.size();i++)
		{			
			QSharedPointer<SurfaceMeshModel> seg = nodes[i]->mMesh;
			QString objId = nodes[i]->mID;

			if (VBO::isVBOSupported())// && !vboCollection.contains(objId))
			{
				Surface_mesh::Vertex_property<Point>  points   = seg->vertex_property<Point>("v:point");
				Surface_mesh::Vertex_property<Point>  vnormals = seg->vertex_property<Point>("v:normal");
				Surface_mesh::Vertex_property<Color>  vcolors  = seg->vertex_property<Color>("v:color");

				QVector<uint> traingles = fillTrianglesList(nodes[i]);

				// Create VBO 
				vboCollection[objId] = VBO( (uint)seg->n_vertices(), points.data(), vnormals.data(), vcolors.data(), traingles);		
			}
		}
	}
}

void MyDesigner::postDraw()
{
	beginUnderMesh();

	// Draw grid
	double gridOpacity = 0.1; 
	if(camera()->position().z < loadedMeshHalfHight) gridOpacity = 0.05;
	glColor4d(0,0,0,gridOpacity);
	glLineWidth(1.0);
	drawGrid(2, Min(10, Max(100, camera()->position().x / skyRadius)));

	// Draw axis
	glPushMatrix();
	glTranslated(-2,-2,0);
	drawAxis(0.8);
	glPopMatrix();

	endUnderMesh();

	glClear(GL_DEPTH_BUFFER_BIT);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	drawViewChanger();
	drawVisualHints(); // Revolve Around Point, line when camera rolls, zoom region
	glPopAttrib();

	drawOSD();
}

void MyDesigner::drawViewChanger()
{
	int viewport[4];
	int scissor[4];

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetIntegerv(GL_SCISSOR_BOX, scissor);

	const int size = 90;
	glViewport(width() - size, 0,size,size);
	glScissor(width() - size, 0,size,size);
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -10, 10);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRotated(-45,1,0,0);glRotated(45,0,0,1);glRotated(180,0,0,1);

	glColor4d(1,1,0.5,0.75); SimpleDraw::DrawCube(Vec3d(0,0,0),0.75f);

	double scale = 1.0;
	
	std::vector<bool> hover(7, false);

	int x = currMousePos2D.x(), y = currMousePos2D.y();
	if(x > width() - size && y > height() - size)
	{
		QPoint p(abs(x - width() + size), abs(y - height() + size));
		QSize box(QSize(size * 0.25, size * 0.25));

		QRect top(QPoint(30,15), box);
		QRect front(QPoint(52,52), box);
		QRect side(QPoint(12,52), box);
		QRect corner(QPoint(32,45), box);
		QRect backside(QPoint(64,18), box);
		QRect back(QPoint(11,20), box);
		QRect bottom(QPoint(30,60), box);

		if(top.contains(p)) hover[0] = true;
		if(front.contains(p)) hover[1] = true;
		if(side.contains(p)) hover[2] = true;
		if(corner.contains(p)) hover[3] = true;
		if(backside.contains(p)) hover[4] = true;
		if(back.contains(p)) hover[5] = true;
		if(bottom.contains(p)) hover[6] = true;
	}

	glColor4d(1,1,0,1);
	Vec3d top (0,0,0.5);if(hover[0]) glColor4d(1,0,0,1);
	SimpleDraw::DrawArrowDirected(top, top, scale,true,true);glColor4d(1,1,0,1);
	Vec3d front (0,0.5,0);if(hover[1]) glColor4d(1,0,0,1);
	SimpleDraw::DrawArrowDirected(front, front, scale,true,true);glColor4d(1,1,0,1);
	Vec3d side (0.5,0,0);if(hover[2]) glColor4d(1,0,0,1);
	SimpleDraw::DrawArrowDirected(side, side, scale,true,true);glColor4d(1,1,0,1);
	Vec3d corner (0.5,0.5,0.5);if(hover[3]) glColor4d(1,0,0,1);
	SimpleDraw::DrawArrowDirected(corner, corner, scale,true,true);glColor4d(1,1,0,1);
	Vec3d backside (-0.5,0,0);if(hover[4]) glColor4d(1,0,0,1);
	SimpleDraw::DrawArrowDirected(backside, backside, scale,true,true);glColor4d(1,1,0,1);
	Vec3d back (0,-0.5,0);if(hover[5]) glColor4d(1,0,0,1);
	SimpleDraw::DrawArrowDirected(back, back, scale,true,true); glColor4d(1,1,0,1);
	Vec3d bottom (0,0,-0.6);if(hover[6]) glColor4d(1,0,0,1);
	SimpleDraw::DrawArrowDirected(bottom, bottom, scale,true,true);glColor4d(1,1,0,1);

	glMatrixMode(GL_PROJECTION);glPopMatrix();
	glMatrixMode(GL_MODELVIEW);glPopMatrix();

	glScissor(scissor[0],scissor[1],scissor[2],scissor[3]);
	glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
}

void MyDesigner::drawOSD()
{
	QStringList selectModeTxt, toolModeTxt;
	selectModeTxt << "Camera" << "Cuboid"<<"Box";
	toolModeTxt << "Freely move camera" << "Press shift to move camera" << "Press shift to move camera";

	int paddingX = 15, paddingY = 5;

	int pixelsHigh = fm->height();
	int lineNum = 0;

	#define padY(l) paddingX, paddingY + (l++ * pixelsHigh*2.5) + (pixelsHigh * 3)

	/* Mode text */
	drawMessage("Select mode: " + selectModeTxt[selectMode], padY(lineNum));
	drawMessage(toolModeTxt[selectMode], width()- 100 - fm->width(toolModeTxt[selectMode])*0.5, paddingY + (pixelsHigh * 3), Vec4d(0.5,0.0,0.5,0.25));

	//if(!gManager) return; //|| !gManager->scaffold
	if(!currGraph) return;

	if(selectMode == CUBOID){
		QVector<Structure::Node*> nodes = currGraph->getSelectedNodes();//activeScaffold()->getSelectedNodes();
		foreach(Structure::Node *n, nodes)
			drawMessage("Cuboid -" + n->mID, padY(lineNum), Vec4d(0,1.0,0,0.25));

	}

	// View changer title
	qglColor(QColor(255,255,180,200));
	renderText(width() - (90*0.5) - (fm->width(viewTitle)*0.5), height() - (fm->height()*0.3),viewTitle);

	// Textual log messages
	for(int i = 0; i < osdMessages.size(); i++){
		int margin = 20; //px
		int x = margin;
		int y = (i * QFont().pointSize() * 1.5f) + margin;

		qglColor(Qt::white);
		renderText(x, y, osdMessages.at(i));
	}

	setForegroundColor(QColor(0,0,0));
	QGLViewer::postDraw();
}


void MyDesigner::drawMessage(QString message, int x, int y, Vec4d &backcolor, Vec4d &frontcolor)
{
	int pixelsWide = fm->width(message);
	int pixelsHigh = fm->height() * 1.5;

	int margin = 20;

	this->startScreenCoordinatesSystem();
	glEnable(GL_BLEND);
	SimpleDraw::drawRoundRect(x, y,pixelsWide + margin, pixelsHigh * 1.5, backcolor, 5);
	this->stopScreenCoordinatesSystem();

	glColor4d(frontcolor[0],frontcolor[1],frontcolor[2],frontcolor[3]);
	drawText( x + margin * 0.5, y - (pixelsHigh * 0.5), message);
}

void MyDesigner::drawCircleFade(Vec4d &color, double radius)
{
	//glDisable(GL_LIGHTING);
	Circle c(Vec3d(0,0,0), Vec3d(0,0,1), radius);
	QVector<Point> pnts = c.getPoints();
	glBegin(GL_TRIANGLE_FAN);
	glColor4d(color[0],color[1],color[2],color[3]); glVertex3d(c.getCenter().x(),c.getCenter().y(),c.getCenter().z());
	glColor4d(color[0],color[1],color[2],0); 
	foreach(Point p, pnts)	glVertex3d(p[0], p[1], p[2]); 
	glVertex3d(pnts[0].x(), pnts[0].y(), pnts[0].z());

	glEnd();
	//glEnable(GL_LIGHTING);
}

void MyDesigner::updateActiveObject()
{
	vboCollection.clear();

	emit(objectUpdated());
}

GraphManager* MyDesigner::activeManager()
{
	return gManager;
}

FdGraph* MyDesigner::activeScaffold()
{
	return gManager->scaffold;
}

bool MyDesigner::isEmpty()
{
	return gManager == NULL;//->scaffold 
}

void MyDesigner::resetView()
{
	setupCamera();
	Geom::AABB aabb = activeScaffold()->computeAABB();

	//camera()->setSceneRadius(activeScaffold()->computeAABB().radius());
	Vec minbound(aabb.bbmin.x(), aabb.bbmin.y(), aabb.bbmin.z());
	Vec maxbound(aabb.bbmax.x(), aabb.bbmax.y(), aabb.bbmax.z());

	camera()->fitBoundingBox( minbound, maxbound);
	camera()->setSceneRadius((maxbound - minbound).norm() * 0.5);
	camera()->setSceneCenter((minbound + maxbound) * 0.5);
	camera()->showEntireScene();
}

void MyDesigner::loadObject()
{
	clearButtons();
	isLoaded = true;

	designWidget->showModel->setChecked(true);
	designWidget->showCuboid->setChecked(true);
	designWidget->showGraph->setChecked(true);
	designWidget->allowScale->setChecked(true);
	designWidget->allowSplit->setChecked(true);

	gManager = NULL;
	mBox = NULL;
	gManager = new GraphManager();
	gManager->loadScaffold();

	fManager = NULL;
	fManager = new FoldManager;

	// Pass the active scaffold to FoldManager
	fManager->setScaffold(activeScaffold());
	Geom::AABB aabb = activeScaffold()->computeAABB();

    if(aabb.radius()>skyRadius*0.1){
		activeScaffold()->normalize(aabb.radius()/skyRadius*10);
	    aabb = activeScaffold()->computeAABB();
	}

	double z = (aabb.bbmax.z() - aabb.bbmin.z())*0.5f;

	if(aabb.center() != Vector3(0,0,z)){
		Vector3 offset = Vector3(0,0,z) - aabb.center();
		activeScaffold()->translate(offset);
	}

	aabb = activeScaffold()->computeAABB();
	mBox = new BBox(aabb.center(), (aabb.bbmax-aabb.bbmin)*0.5f);
	scalePercent = 0.0;

	loadedMeshHalfHight = (aabb.bbmax.z() - aabb.bbmin.z()) * -0.01;

	// Set scaffold to display
	currGraph = activeScaffold()->deepClone();

	// Set camera
	resetView();

	// Update the object
	updateActiveObject();

	setManipulatedFrame(activeFrame);

	updateGL();
}

void MyDesigner::setActiveObject(GraphManager *gm)
{
	// Delete the original object
	if (activeManager()&&activeScaffold())
		emit(objectDiscarded());

	// Setup the new object
	gManager = gm;

	// Change title of scene
	setWindowTitle(activeScaffold()->path);

	// Set camera
	resetView();

	// Update the object
	updateActiveObject();

	//SaveUndo();

	emit(objectInserted());
}

void MyDesigner::newScene()
{
	setSelectMode(SELECT_NONE);
	this->setCursor(QCursor(Qt::ArrowCursor));
	selectCameraMode();
	clearButtons();
	
	if(activeManager())
		emit( objectDiscarded());

	designWidget->showModel->setChecked(true);
	designWidget->showCuboid->setChecked(true);
	designWidget->showGraph->setChecked(true);
	designWidget->allowScale->setChecked(true);
	designWidget->allowSplit->setChecked(true);

	gManager = NULL;
	mBox = NULL;
	
	// Update the object
	updateActiveObject();

	isMousePressed = false;
	isLoaded = false;

	fManager = NULL;
	fManager = new FoldManager;

	currGraph = NULL;

	updateGL();
}

void MyDesigner::mousePressEvent( QMouseEvent* e )
{
	QGLViewer::mousePressEvent(e);

	if((e->button() == Qt::LeftButton) && !(e->modifiers() & Qt::ShiftModifier)) 
	{
		this->startMousePos2D = e->pos();
		camera()->convertClickToLine(e->pos(), startMouseOrigin, startMouseDir);

		if(selectMode == BOX && mBox)
		{
			Point start(startMouseOrigin[0], startMouseOrigin[1],startMouseOrigin[2]);
			Vec3d dir(startMouseDir[0], startMouseDir[1], startMouseDir[2]);
			mBox->selectFace(start,dir);
			if(mBox->axisID < 0 || mBox->selPlaneID < 0){
				selectMode = BOX;
				this->displayMessage("* No face has been selected. LEFT CLICK to select a face to push in. *");
			}
			else{
				this->displayMessage("* Setting up fold handle. Please wait!!! *", 5000);
				fManager->foldAlongAxis(mBox->axisID);
				fManager->createDcGraphs();
			    this->displayMessage("* Done!!! Please CTRL + SCROLL THE MOUSE to squeeze the box. *", 5000);
				scalePercent = 0.0;
			}
		    updateGL();
		}

		if(selectMode == CUBOID && currGraph)
		{

		}
	}

	if(!isMousePressed)
	{
		// Set constraints
		if(selectMode == BOX)
		{

		}

		if(selectMode == CUBOID)
		{

		}
	}

	isMousePressed = true;
}


void MyDesigner::mouseReleaseEvent( QMouseEvent* e )
{
	QGLViewer::mouseReleaseEvent(e);

	isMousePressed = false;

	// View changer box area
	double scale = 90;
	int x = e->pos().x(), y = e->pos().y();
	if(x > width() - scale && y > height() - scale && gManager)
	{
		QPoint p(abs(x - width() + scale), abs(y - height() + scale));
		Geom::AABB aabb = currGraph->computeAABB();

		double meshHeight = aabb.bbmax.z() - aabb.bbmin.z();
		double meshLength = aabb.bbmax.y() - aabb.bbmin.y();
		double meshWidth = aabb.bbmax.x() - aabb.bbmin.x();

		QSize box(QSize(scale * 0.25, scale * 0.25));

		QRect top(QPoint(30,15), box);
		QRect front(QPoint(52,52), box);
		QRect side(QPoint(12,52), box);
		QRect corner(QPoint(32,45), box);
		QRect backside(QPoint(64,18), box);
		QRect back(QPoint(11,20), box);
		QRect bottom(QPoint(30,70), box);

		if(top.contains(p)){
			qglviewer::Frame f(Vec(0,0,3*meshHeight), qglviewer::Quaternion());
			camera()->interpolateTo(f,0.25);camera()->setType(Camera::ORTHOGRAPHIC);
			viewTitle = "Top";
		}

		if(bottom.contains(p)){
			qglviewer::Frame f(Vec(0,0,-3*meshHeight), qglviewer::Quaternion());
			f.rotate(qglviewer::Quaternion(Vec(1,0,0), M_PI));
			camera()->interpolateTo(f,0.25);camera()->setType(Camera::ORTHOGRAPHIC);
			viewTitle = "Bottom";
		}

		if(front.contains(p)){
			qglviewer::Frame f(Vec(3*meshLength,0,0), qglviewer::Quaternion(Vec(0,0,1),Vec(1,0,0)));
			f.rotate(qglviewer::Quaternion(Vec(0,0,1), M_PI / 2.0));
			camera()->interpolateTo(f,0.25);camera()->setType(Camera::ORTHOGRAPHIC);
			viewTitle = "Front";
		}

		if(side.contains(p)){
			qglviewer::Frame f(Vec(0,-3*meshWidth,0), qglviewer::Quaternion(Vec(0,0,1),Vec(0,-1,0)));
			camera()->interpolateTo(f,0.25);camera()->setType(Camera::ORTHOGRAPHIC);
			viewTitle = "Side";
		}
		
		if(backside.contains(p)){
			qglviewer::Frame f(Vec(0,3*meshWidth,0), qglviewer::Quaternion(Vec(0,0,1),Vec(0,1,0)));
			f.rotate(qglviewer::Quaternion(Vec(0,0,1), M_PI));
			camera()->interpolateTo(f,0.25);camera()->setType(Camera::ORTHOGRAPHIC);
			viewTitle = "Back-side";
		}
		 
		if(back.contains(p)){
			qglviewer::Frame f(Vec(-3*meshLength,0,0), qglviewer::Quaternion(Vec(0,0,-1),Vec(1,0,0)));
			f.rotate(qglviewer::Quaternion(Vec(0,0,-1), M_PI / 2.0));
			camera()->interpolateTo(f,0.25);camera()->setType(Camera::ORTHOGRAPHIC);
			viewTitle = "Back";
		}

		if(corner.contains(p)){
			double mx = 2*Max(meshLength, Max(meshWidth, meshHeight));
			qglviewer::Frame f(Vec(mx,-mx,mx), qglviewer::Quaternion());
			f.rotate(qglviewer::Quaternion(Vec(0,0,1), M_PI / 4.0));
			f.rotate(qglviewer::Quaternion(Vec(1,0,0), M_PI / 3.3));
			camera()->interpolateTo(f,0.25);camera()->setType(Camera::PERSPECTIVE);
			viewTitle = "View";
		}
	}
	else
	{
		if( selectMode != SELECT_NONE) //&& activeOffset)
		{
			//updateOffset();
		}
	}

	updateGL();
}

void MyDesigner::mouseMoveEvent( QMouseEvent* e )
{
	QGLViewer::mouseMoveEvent(e);
	currMousePos2D = e->pos();
	camera()->convertClickToLine(currMousePos2D, currMouseOrigin, currMouseDir);

	if(e->buttons() & Qt::LeftButton)
	{
		isMousePressed = true;
	}

	double scale = 90;
	int x = e->pos().x(), y = e->pos().y();
	if(x > width() - scale && y > height() - scale)
	{
		updateGL();
	}

}

void MyDesigner::wheelEvent( QWheelEvent* e )
{
	QGLViewer::wheelEvent(e);
 
	if(selectMode == BOX && (e->modifiers() & Qt::ControlModifier))
	{
		if(mBox->selPlaneID >= 0){
			double factor =  0.05 * (e->delta() / 120.0);
			scalePercent += factor;
			if(fabs(scalePercent) >= 1){
				scalePercent -= factor;
				factor = 0.0;
			}
			mBox->deform(factor);
			mBox->getBoxFaces();
		
			fManager->foldAll();
			fManager->generateFdKeyFrames();

		    emit(resultsGenerated());
		}
		else
	        this->displayMessage("* Fail to push in the right direction *", 5000);
		updateGL();
	}		
}

void MyDesigner::keyPressEvent( QKeyEvent *e )
{
	if(e->key() == Qt::Key_L)
	{
		this->loadObject();
	}

	if(e->key() == Qt::Key_V)		selectCameraMode();
	if(e->key() == Qt::Key_C)	    selectCuboidMode();
	if(e->key() == Qt::Key_A)		selectAABBMode();

	updateGL();

	if(e->key() != Qt::Key_Space) // disable fly mode..
		QGLViewer::keyPressEvent(e);
}

void MyDesigner::saveObject()
{
	if(activeManager() && activeScaffold())
		gManager->saveScaffold();
}

void MyDesigner::beginUnderMesh()
{
	if(isEmpty()) return;

	glPushMatrix();
	glTranslated(0,0, loadedMeshHalfHight);
}

void MyDesigner::endUnderMesh()
{
	if(isEmpty()) return;
	glPopMatrix();
}

void MyDesigner::drawWithNames()
{
	if(isEmpty()) return;
	if(!currGraph) return;

	currGraph->drawWithNames();
	//activeScaffold()->drawWithNames();
}

void MyDesigner::postSelection( const QPoint& point )
{
	if(currMousePos2D.x() > width() - 90 && currMousePos2D.y() > height() - 90)
		return;

	//int selected = selectedName();
	if (selectMode == CUBOID && currGraph)
	{
		int nidx = selectedName();

		Structure::Node* sn = currGraph->getNode(nidx);//activeScaffold()->getNode(nidx);
		if (sn)
		{
			currGraph->selectNode(nidx);
			//activeScaffold()->selectNode(nidx);
		}
	}

	updateGL();
}

void MyDesigner::setSelectMode( SelectMode toMode )
{
	this->selectMode = toMode;
}


void MyDesigner::selectCuboidMode()
{
	clearButtons();
	if(!currGraph){
		this->displayMessage("* Please import an object first *", 1000);
		return;
	}
	setManipulatedFrame(activeFrame);

	//if(!activeScaffold()) return;

	setMouseBinding(Qt::LeftButton, SELECT);
	setMouseBinding(Qt::ShiftModifier | Qt::LeftButton, CAMERA, ROTATE);

	setSelectMode(CUBOID);
	currGraph->showCuboids(true);
	//activeScaffold()->showCuboids(true);
	this->setCursor(QCursor(QPixmap(":/Resources/push.png"), 0, 32));
	this->displayMessage("* Left click to select a node *", 5000);
	selectTool();
}


void MyDesigner::selectCameraMode()
{
	clearButtons();
	setManipulatedFrame(activeFrame);

	//if(!activeManager()) return;
	if(!currGraph) return;

	designWidget->selectCameraButton->setChecked(true);
	
	//setMouseBinding(Qt::ShiftModifier | Qt::LeftButton, SELECT);
	setMouseBinding(Qt::LeftButton, CAMERA, ROTATE);
	currGraph->showCuboids(designWidget->showCuboid->isChecked());
	//activeScaffold()->showCuboids(designWidget->showCuboid->isChecked());
	this->setCursor(QCursor(Qt::ArrowCursor));
	setSelectMode(SELECT_NONE);
	updateGL();
}

void MyDesigner::selectAABBMode()
{
	clearButtons();
	if(!currGraph){
		this->displayMessage("* Please import an object first *", 1000);
		return;
	}

	//setMouseBinding(Qt::RightButton, SELECT);
	setMouseBinding(Qt::ShiftModifier | Qt::LeftButton, CAMERA, ROTATE);
	currGraph->showCuboids(designWidget->showCuboid->isChecked());
	//activeScaffold()->showCuboids(designWidget->showCuboid->isChecked());
	this->setCursor(QCursor(QPixmap(":/Resources/push.png"), 0, 32));
	setSelectMode(BOX);
    selectTool();
	this->displayMessage("* Left click to select a face to push in *", 5000);
}

void MyDesigner::selectTool()
{
	if(gManager == NULL) return;

	if(selectMode == BOX) designWidget->pushButton->setChecked(true);
	if(selectMode == CUBOID) designWidget->selectCuboidButton->setChecked(true);

	updateGL();
}

void MyDesigner::clearButtons()
{
	designWidget->selectCameraButton->setChecked(false);
	designWidget->selectCuboidButton->setChecked(false);
	designWidget->pushButton->setChecked(false);
}

void MyDesigner::print( QString message, long age )
{
	osdMessages.enqueue(message);
	timerScreenText->start(age);
	updateGL();
}

void MyDesigner::dequeueLastMessage()
{
	if(!osdMessages.isEmpty()){
		osdMessages.dequeue();
		updateGL();
	}
}

void MyDesigner::showCuboids(int state)
{
	if(isEmpty()) return;
	if(!currGraph) return;
	//gManager->scaffold->showCuboids(state);
	currGraph->showCuboids(state);
	updateGL();
}
void MyDesigner::showGraph(int state)
{
	if(isEmpty()) return;
	if(!currGraph) return;
	//gManager->scaffold->showScaffold(state);
	currGraph->showScaffold(state);
	updateGL();
}

void MyDesigner::showModel(int state)
{
	if(isEmpty()) return;
	if(!currGraph) return;
	//gManager->scaffold->showMeshes(state);
	currGraph->showMeshes(state);
	isShow = (state == Qt::Checked);
	updateGL();
}

void MyDesigner::setScalable(int state)
{
	if(isEmpty()) return;
	if(!currGraph) return;
	bool isScalable = (state == Qt::Checked);
	QVector <Structure::Node *> selectedNodes = currGraph->getSelectedNodes();
	foreach(Structure::Node *n, selectedNodes){
		activeScaffold()->getNode(n->mID)->properties["isScalable"] = isScalable;
	}
	//foreach(Structure::Node *n, selectedNodes)
	//	n->properties["isScalable"] = isScalable;
}

void MyDesigner::setSplittable(int state)
{
	if(isEmpty()) return;
	if(!currGraph) return;
	bool isSplittable = (state == Qt::Checked);
	QVector <Structure::Node *> selectedNodes = currGraph->getSelectedNodes();
	foreach(Structure::Node *n, selectedNodes){
		activeScaffold()->getNode(n->mID)->properties["isScalable"] = isSplittable;
	}
	/*foreach(Structure::Node *n, selectedNodes)
	n->properties["isSplittable"] = isSplittable;*/
}
