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
#include "MyAnimator.h"

using namespace Geom;

// Misc.
#include "UiUtility/sphereDraw.h"
#include "UiUtility/drawRoundRect.h"
#include "UiUtility/drawPlane.h"
#include "UiUtility/drawCube.h"
#include "UiUtility/SimpleDraw.h"

#include <QTime>
extern QTime * globalTimer;

MyAnimator::MyAnimator(Ui::EvaluateWidget * useAnimWidget, QWidget * parent /*= 0*/ ) : QGLViewer(parent)
{
	this->evalWidget = useAnimWidget;

	skyRadius = 1.0;
	isMousePressed = false;
	loadedMeshHalfHight = 1.0;

	isShow = true;

	mCurrConfigId = 0;
	mCurrGraphId = 0;

	fm = new QFontMetrics(QFont());

	connect(this, SIGNAL(objectInserted()), SLOT(updateGL()));

	activeFrame = new ManipulatedFrame();
	setManipulatedFrame(activeFrame);

	VideoToolbar *vti = new VideoToolbar;
	this->evalWidget->viewerAreaLayout->addWidget(vti);

	// TEXT ON SCREEN
	timerScreenText = new QTimer(this);
	connect(timerScreenText, SIGNAL(timeout()), SLOT(dequeueLastMessage()));
	connect(camera()->frame(), SIGNAL(manipulated()), SLOT(cameraMoved()));

	//Animation
	isPlaying = false;
	connect(vti, SIGNAL(vti->valueChanged(int)), SLOT(toggleSlider(int)));
	connect(this, SIGNAL(setSliderValue(int)), vti, SLOT(vti->sliderChanged(int)));
	connect(vti->ui->playButton, SIGNAL(clicked()), SLOT(togglePlay()));

	this->setMouseTracking(true);

	viewTitle = "View";
}

void MyAnimator::init()
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

void MyAnimator::setupLights()
{
	GLfloat lightColor[] = {0.9f, 0.9f, 0.9f, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
}

void MyAnimator::setupCamera()
{
	camera()->setSceneRadius(10.0);
	camera()->showEntireScene();
	camera()->setUpVector(Vec(0,0,1));
	camera()->setPosition(Vec(2,-2,2));
	camera()->lookAt(Vec());
}

void MyAnimator::cameraMoved()
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

void MyAnimator::preDraw()
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
	drawSolidSphere(skyRadius,30,30, true, true); // Sky dome

	beginUnderMesh();
	double floorOpacity = 0.25; 
	if(camera()->position().z < loadedMeshHalfHight) floorOpacity = 0.05;
	drawCircleFade(Vec4d(0,0,0,floorOpacity), 4); // Floor
	drawShadows();
	endUnderMesh();

	glClear(GL_DEPTH_BUFFER_BIT);
}

void MyAnimator::drawShadows()
{
	if(!gManager || camera()->position().z < loadedMeshHalfHight) return;

	// Compute shadow matrix
	GLfloat floorShadow[4][4];
	GLfloat groundplane[4];
	findPlane(groundplane, Vec3d(0,0,0), Vec3d(-1,0,0), Vec3d(-1,-1,0));
	GLfloat lightpos[4] = {0.0,0.0,8,1};
	shadowMatrix(floorShadow,groundplane, lightpos);

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

void MyAnimator::draw()
{
	// No object to draw
	if (isEmpty()) return;

	// The main object
	if(activeScaffold()){
		drawObject();
		activeScaffold()->draw();
	}

	glEnable(GL_BLEND);
	//drawDebug();
}

void MyAnimator::drawDebug()
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

void MyAnimator::drawObject()
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
		//gManager->scaffold->draw();
		activeScaffold()->draw();
	}
}

void MyAnimator::drawObjectOutline()
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

QVector<uint> MyAnimator::fillTrianglesList(FdNode* n)
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

void MyAnimator::updateVBOs()
{
	if(gManager && activeScaffold())
	{
		// Create VBO for each segment if needed
		QVector<FdNode*> nodes = activeScaffold()->getFdNodes();
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

void MyAnimator::postDraw()
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

void MyAnimator::drawViewChanger()
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

void MyAnimator::drawOSD()
{
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


void MyAnimator::drawMessage(QString message, int x, int y, Vec4d &backcolor, Vec4d &frontcolor)
{
	int pixelsWide = fm->width(message);
	int pixelsHigh = fm->height() * 1.5;

	int margin = 20;

	this->startScreenCoordinatesSystem();
	glEnable(GL_BLEND);
	drawRoundRect(x, y,pixelsWide + margin, pixelsHigh * 1.5, backcolor, 5);
	this->stopScreenCoordinatesSystem();

	glColor4d(frontcolor[0],frontcolor[1],frontcolor[2],frontcolor[3]);
	drawText( x + margin * 0.5, y - (pixelsHigh * 0.5), message);
}

void MyAnimator::drawCircleFade(Vec4d &color, double radius)
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

void MyAnimator::updateActiveObject()
{
	vboCollection.clear();

	emit(objectUpdated());
}

GraphManager* MyAnimator::activeManager()
{
	return gManager;
}

FdGraph* MyAnimator::activeScaffold()
{
	//return gManager->scaffold;
	return mGraphs[mCurrConfigId][mCurrGraphId];
}

bool MyAnimator::isEmpty()
{
	//return gManager == NULL;//->scaffold 
	return (mGraphs.size() == 0);
}

void MyAnimator::resetView()
{
	setupCamera();
	if(isEmpty()) return;
	camera()->setSceneRadius(activeScaffold()->computeAABB().radius());
	camera()->showEntireScene();
}

void MyAnimator::loadConfig(int configId)
{
	//gManager = NULL;
	//gManager = new GraphManager();
	//gManager->loadScaffold();
    
	if(isEmpty()) return;
	mCurrConfigId = configId;
	setManipulatedFrame(activeFrame);

	updateGL();
}

void MyAnimator::setActiveObject(GraphManager *gm)
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

void MyAnimator::newScene()
{
	if(activeManager())
		emit( objectDiscarded());

	//selection.clear();

	gManager = NULL;
	setWindowTitle(" ");
	// Update the object
	updateActiveObject();

	//SaveUndo();
	isMousePressed = false;
	updateGL();
}

void MyAnimator::mousePressEvent( QMouseEvent* e )
{
	QGLViewer::mousePressEvent(e);

	isMousePressed = true;
}


void MyAnimator::mouseReleaseEvent( QMouseEvent* e )
{
	QGLViewer::mouseReleaseEvent(e);

	isMousePressed = false;

	// View changer box area
	double scale = 90;
	int x = e->pos().x(), y = e->pos().y();
	if(x > width() - scale && y > height() - scale && gManager)
	{
		QPoint p(abs(x - width() + scale), abs(y - height() + scale));
		Geom::AABB aabb = activeScaffold()->computeAABB();

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

	updateGL();
}

void MyAnimator::mouseMoveEvent( QMouseEvent* e )
{
	QGLViewer::mouseMoveEvent(e);
	currMousePos2D = e->pos();
	double scale = 90;
	int x = e->pos().x(), y = e->pos().y();
	if(x > width() - scale && y > height() - scale)
	{
		updateGL();
	}

}

void MyAnimator::wheelEvent( QWheelEvent* e )
{
	QGLViewer::wheelEvent(e);

	updateGL();
}

void MyAnimator::keyPressEvent( QKeyEvent *e )
{
	updateGL();

	if(e->key() != Qt::Key_Space) // disable fly mode..
		QGLViewer::keyPressEvent(e);
}

void MyAnimator::beginUnderMesh()
{
	if(isEmpty()) return;

	glPushMatrix();
	glTranslated(0,0, loadedMeshHalfHight);
}

void MyAnimator::endUnderMesh()
{
	if(isEmpty()) return;
	glPopMatrix();
}

void MyAnimator::drawWithNames()
{
	activeScaffold()->drawWithNames();
}

void MyAnimator::print( QString message, long age )
{
	osdMessages.enqueue(message);
	timerScreenText->start(age);
	updateGL();
}

void MyAnimator::dequeueLastMessage()
{
	if(!osdMessages.isEmpty()){
		osdMessages.dequeue();
		updateGL();
	}
}

void MyAnimator::startAnimation()
{
	QGLViewer::startAnimation();
}

void MyAnimator::stopAnimation()
{
	QGLViewer::stopAnimation();
}

void MyAnimator::animate()
{
	for(int i = 0; i < mGraphs[mCurrConfigId].size(); i++){
		mCurrGraphId = i;
		updateGL();
		emit setSliderValue(100 * (double(i) / mGraphs[mCurrConfigId].size()));
	}
}

void MyAnimator::toggleSlider(int frameId)
{
	mCurrGraphId = frameId;
	updateGL();
}

void MyAnimator::togglePlay()
{
	QString label = vti->ui->playButton->text();

	QString playLabel = vti->ui->playLabel->text();
	QString pauseLabel = vti->ui->pauseLabel->text();

	if(label != pauseLabel)
	{
		vti->ui->playButton->setText(pauseLabel);
		isPlaying = true;
		startAnimation();
	}
	else
	{
		vti->ui->playButton->setText(playLabel);
		isPlaying = false;
		stopAnimation();
	}
}