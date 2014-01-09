#include "QuickMeshViewer.h"
#include <QFileInfo>

#include "AABB.h"

QuickMeshViewer::QuickMeshViewer( QWidget * parent /*= 0*/ ) :QGLViewer(parent)
{
	this->setMaximumSize(300,220);

	this->isActive = false;
	this->isLoading = false;

	mGraph = NULL;
	
	// Could cause crashes..
	connect(this, SIGNAL(graphLoaded()), SLOT(updateGL()));
}

QuickMeshViewer::~QuickMeshViewer()
{
	/*if(mGraph)
	delete mGraph;*/
}

void QuickMeshViewer::init()
{
	QGLViewer::init();

	//setBackgroundColor(QColor(50,60,60));

	// Light
	GLfloat lightColor[] = {0.9f, 0.9f, 0.9f, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);

	// Material
	float mat_ambient[] = {0.1745f, 0.01175f, 0.01175f, 1.0f};
	float mat_diffuse[] = {0.65f, 0.045f, 0.045f, 1.0f};
	float mat_specular[] = {0.09f, 0.09f, 0.09f, 1.0f};
	float high_shininess = 100;

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, high_shininess);

	// Camera
	camera()->setType(Camera::ORTHOGRAPHIC);

	setupCamera();
}

void QuickMeshViewer::draw()
{
	if(!this->isActive) return;

	glEnable(GL_MULTISAMPLE);
	
	if(mGraph){
		// Temporarily set
		mGraph->showCuboids(false);
		mGraph->showScaffold(false);
		mGraph->showMeshes(true);
		mGraph->draw();
	}

	glEnable(GL_BLEND);
}

void QuickMeshViewer::preDraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	glClear(GL_DEPTH_BUFFER_BIT);
}

void QuickMeshViewer::postDraw()
{
	if(!this->isActive) return;

	if(isLoading){
		glColor3d(1,1,1);
		//renderText(8, 15, "Loading...");
	}
	else
	{
		glColor4d(1,1,1,0.5);
		//renderText(8, 15, mGraph->mID);
	}

	if(this->hasFocus())
	{
		int w = width(), h = height();
		startScreenCoordinatesSystem();
		glColor3d(0.8,0.2,0.2);
		glLineWidth(10);
		glBegin(GL_LINE_STRIP);
		glVertex3dv(Vec(0,0,0));
		glVertex3dv(Vec(w,0,0));
		glVertex3dv(Vec(w,h,0));
		glVertex3dv(Vec(0,h,0));
		glVertex3dv(Vec(0,0,0));
		glEnd();
		stopScreenCoordinatesSystem();
	}

	QGLViewer::postDraw();
}

void QuickMeshViewer::focusInEvent( QFocusEvent * event )
{
	emit(gotFocus(this));
}

void QuickMeshViewer::clearGraph()
{
	mGraph = NULL;
	isActive = false;
	isLoading = false;
	updateGL();
}

void QuickMeshViewer::setGraph(FdGraph *graph)
{
	isLoading = true;
	mGraph = graph;
	isLoading = false;
	isActive = true;

	Geom::AABB aabb = graph->computeAABB();
	Vec center(aabb.center().x(), aabb.center().y(), aabb.center().z());
	setSceneRadius((aabb.bbmax - aabb.bbmin).norm()*0.5f);
    setSceneCenter(center);

	resetView();

	//updateGL();
	emit(graphLoaded());
}

void QuickMeshViewer::loadGraph( QString fileName )
{
	isLoading = true;
	mGraph->loadFromFile(fileName);
	isLoading = false;

	isActive = true;

	resetView();

	updateGL();
	emit(graphLoaded());
}

void QuickMeshViewer::resetView()
{
	setupCamera();
	if(!mGraph) return;

	Geom::AABB aabb = mGraph->computeAABB();

	//camera()->setSceneRadius(activeScaffold()->computeAABB().radius());
	Vec minbound(aabb.bbmin.x(), aabb.bbmin.y(), aabb.bbmin.z());
	Vec maxbound(aabb.bbmax.x(), aabb.bbmax.y(), aabb.bbmax.z());

	camera()->fitBoundingBox( minbound, maxbound);
	camera()->setSceneRadius((maxbound - minbound).norm() * 0.5);
	camera()->setSceneCenter((minbound + maxbound) * 0.5);
	camera()->showEntireScene();
}

void QuickMeshViewer::setupCamera()
{
	camera()->setSceneRadius(2.0);
	camera()->showEntireScene();
	camera()->setUpVector(Vec(0,0,1));
	camera()->setPosition(Vec(2,-2,2));
	camera()->lookAt(Vec());
}
