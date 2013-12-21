#include "Circle.h"
#include "UiUtility/QManualDeformer.h"
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
//std::vector<Vector3> segPlane;
//Line intLine;
//Point intPnt;
// Misc.
#include "UiUtility/sphereDraw.h"
#include "UiUtility/drawRoundRect.h"
#include "UiUtility/drawPlane.h"
#include "UiUtility/drawCube.h"
#include "UiUtility/SimpleDraw.h"

#include "MyDesigner.h"

#include <QTime>
extern QTime * globalTimer;

MyDesigner::MyDesigner( Ui::DesignWidget * useDesignWidget, QWidget * parent /*= 0*/ ) : QGLViewer(parent)
{
	this->designWidget = useDesignWidget;

	//Transformation widget
	//transWidget->setupUi(this);
	//rotWidget->setupUi(this);
	//scaleWidget->setupUi(this);

	defCtrl = NULL;

	selectMode = SELECT_NONE;
	transformMode = NONE_MODE;
	skyRadius = 1.0;
	gManager = NULL;
	isMousePressed = false;
	loadedMeshHalfHight = 1.0;

	isShow = true;

	fm = new QFontMetrics(QFont());

	connect(this, SIGNAL(objectInserted()), SLOT(updateGL()));

	activeFrame = new ManipulatedFrame();
	setManipulatedFrame(activeFrame);

	// TEXT ON SCREEN
	timerScreenText = new QTimer(this);
	connect(timerScreenText, SIGNAL(timeout()), SLOT(dequeueLastMessage()));
    connect(camera()->frame(), SIGNAL(manipulated()), SLOT(cameraMoved()));

	connect(designWidget->showCuboid, SIGNAL(stateChanged(int)),  SLOT(showCuboids(int)));
	connect(designWidget->showGraph, SIGNAL(stateChanged(int)),  SLOT(showGraph(int)));
	connect(designWidget->showModel, SIGNAL(stateChanged(int)), SLOT(showModel(int)));

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
	drawSolidSphere(skyRadius,30,30, true, true); // Sky dome

	beginUnderMesh();
	double floorOpacity = 0.25; 
	if(camera()->position().z < loadedMeshHalfHight) floorOpacity = 0.05;
	drawCircleFade(Vec4d(0,0,0,floorOpacity), 4); // Floor
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

void MyDesigner::draw()
{
	// No object to draw
	if (isEmpty()) return;

	// The main object
	if(gManager->scaffold){
		drawObject();
		gManager->scaffold->draw();
		/*Geom::AABB aabb = gManager->scaffold->computeAABB();
		mBox = new BBox(aabb.center(), (aabb.bbmax-aabb.bbmin)*0.5f);
		mBox->draw();*/
	}

	if(selectMode == BOX && gManager->scaffold){
		Geom::AABB aabb = gManager->scaffold->computeAABB();
		if(mBox == NULL)
			mBox = new BBox(aabb.center(), (aabb.bbmax-aabb.bbmin)*0.5f);
		mBox->draw();
	}

	// Draw debug geometries
	//activeObject()->drawDebug();

	//if(transformMode == SPLIT_MODE && segPlane.size()){	
	//	//SimpleDraw::DrawArrow(Vec3d(startMouseOrigin[0],startMouseOrigin[1],startMouseOrigin[2]), 
	//	//	                  Vec3d((startMouseDir+startMouseOrigin)[0],(startMouseDir+startMouseOrigin)[1],(startMouseDir+startMouseOrigin)[2]), true, true, 0.3f);
	//	SimpleDraw::DrawSphere(intPnt, 0.01f);
	//	//SimpleDraw::DrawArrow(intLine.a, intLine.b, true, true, 0.1f);
	//	SimpleDraw::DrawSquare(segPlane[0], segPlane[1], segPlane[2], segPlane[3],true,1.0f,Vec4d(0.8,0.65,0.2,1));
	//}

	glEnable(GL_BLEND);

	//drawDebug();
}

void MyDesigner::drawTool()
{
	if(!selection.size()) return;

	double toolScale = 0.4;

	switch(transformMode){
	case NONE_MODE: break;
	case TRANSLATE_MODE:        
		{
			glPushMatrix();
			ManipulatedFrame * m = manipulatedFrame();
			const GLdouble * mat = m->matrix();
			glMultMatrixd(mat);
			glColor3f(1,1,0);
			glRotated(-90,0,0,1);
			drawAxis(toolScale);
			SimpleDraw::DrawSolidBox(Vec3d(0),0.1,0.1,0.1, 1,1,0,1);
			glPopMatrix();
			break;
		}
	case ROTATE_MODE:
		{
			glDisable(GL_LIGHTING);
			glPushMatrix();
			glMultMatrixd(defCtrl->getFrame()->matrix());

			Circle c(Vec3d(0,0,0), Vec3d(0,0,1),toolScale * 0.75);
			c.draw(1,Vec4d(1,1,0,1)); c.draw(3,Vec4d(0,0,0,1));
			glRotated(90,0,1,0);
			c.draw(1,Vec4d(1,1,0,1)); c.draw(3,Vec4d(0,0,0,1));
			glRotated(90,1,0,0);
			c.draw(1,Vec4d(1,1,0,1)); c.draw(3,Vec4d(0,0,0,1));
			
			glDisable(GL_DEPTH_TEST);
			glColor4d(1,1,0,0.1);
			drawSolidSphere(toolScale* 0.75, 20,20);
			glEnable(GL_DEPTH_TEST);

			glPopMatrix();
			glEnable(GL_LIGHTING);
			break;
		}
	case SCALE_MODE:
		{
			glPushMatrix();
			glMultMatrixd(defCtrl->getFrame()->matrix());

			Vec3d delta = scaleDelta;

			toolScale *= 0.25;

			glEnable(GL_LIGHTING);
			glColor4d(1,1,0,1);
			if(delta.x() != 1) SimpleDraw::DrawArrowDirected(Vec3d(0), Vec3d(1,0,0),0.2);
			if(delta.y() != 1) SimpleDraw::DrawArrowDirected(Vec3d(0), Vec3d(0,-1,0),0.2);
			if(delta.z() != 1) SimpleDraw::DrawArrowDirected(Vec3d(0), Vec3d(0,0,1),0.2);
			glDisable(GL_LIGHTING);

			delta.x() = abs(delta.x());
			delta.y() = abs(delta.y());
			delta.z() = abs(delta.z());
			glScaled(delta.x(), delta.y(), delta.z());

			Circle c1(Vec3d(toolScale*0.75,-toolScale*0.75,0), Vec3d(0,0,1),toolScale, 4);
			c1.drawFilled(Vec4d(1,1,0,0.2), 2, Vec4d(0,0,0,1));

			Circle c2(Vec3d(toolScale*0.75,0,toolScale*0.75), Vec3d(0,1,0),toolScale, 4);
			c2.drawFilled(Vec4d(1,1,0,0.2), 2, Vec4d(0,0,0,1));

			Circle c3(Vec3d(0,-toolScale*0.75,toolScale*0.75), Vec3d(1,0,0),toolScale, 4);
			c3.drawFilled(Vec4d(1,1,0,0.2), 2, Vec4d(0,0,0,1));

			glPopMatrix();
			glEnable(GL_LIGHTING);
			break;
		}
	case SPLIT_MODE:
		{
			break;
		}
	}
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
		gManager->scaffold->draw();
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
	if(gManager && gManager->scaffold)
	{
		// Create VBO for each segment if needed
	    QVector<FdNode*> nodes = gManager->scaffold->getFdNodes();
		for (int i=0;i<nodes.size();i++)
		{			
			QSharedPointer<SurfaceMeshModel> seg = nodes[i]->mMesh;
			QString objId = nodes[i]->id;

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
	drawTool();
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
	selectModeTxt << "Camera" << "Mesh" << "Vertex" << "Edge" 
		<< "Face" << "Primitive" << "Curve"<<"Box";
	toolModeTxt << "None" << "Move" << "Rotate" << "Scale"<<"Split";

	int paddingX = 15, paddingY = 5;

	int pixelsHigh = fm->height();
	int lineNum = 0;

	#define padY(l) paddingX, paddingY + (l++ * pixelsHigh*2.5) + (pixelsHigh * 3)

	/* Mode text */
	drawMessage("Select mode: " + selectModeTxt[this->selectMode], padY(lineNum));

	/*if(!ctrl()) return;

	if(selectMode == CONTROLLER || selectMode == CONTROLLER_ELEMENT)
	{
	QString primId = "None";
	if(ctrl()->getSelectedPrimitive())
	primId = ctrl()->getSelectedPrimitive()->id;
	drawMessage("Primitive - " + primId, padY(lineNum), Vec4d(1.0,1.0,0,0.25));
	}*/

	/*if(selectMode == CONTROLLER_ELEMENT && ctrl()->getSelectedPrimitive())
	{
		QString curveId = "None";
		if(ctrl()->getSelectedPrimitive()->selectedCurveId >= 0)
			curveId = QString::number(ctrl()->getSelectedPrimitive()->selectedCurveId);
		drawMessage("Curve - " + curveId, padY(lineNum), Vec4d(0,1.0,0,0.25));
	}*/

	if(selectMode == BOX)
		drawMessage("Push AABB", padY(lineNum), Vec4d(0,1.0,0,0.25));

	if(transformMode != NONE_MODE)
	{
		drawMessage("Tool: " + toolModeTxt[transformMode], padY(lineNum), Vec4d(1.0,1.0,0,0.25));
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
	drawRoundRect(x, y,pixelsWide + margin, pixelsHigh * 1.5, backcolor, 5);
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
	//glVertex3d(pnts.front().x(), pnts.front().y(),pnts.front().y());
	glEnd();
	//glEnable(GL_LIGHTING);
}

void MyDesigner::updateActiveObject()
{
	vboCollection.clear();

	emit(objectUpdated());
}

GraphManager* MyDesigner::activeObject()
{
	return gManager;
}

bool MyDesigner::isEmpty()
{
	return gManager == NULL;
}

void MyDesigner::resetView()
{
	setupCamera();
	camera()->setSceneRadius(gManager->scaffold->computeAABB().radius());
	camera()->showEntireScene();
}

void MyDesigner::loadObject()
{
	clearButtons();
	selection.clear();

	gManager = NULL;
	mBox = NULL;
	gManager = new GraphManager();
	gManager->loadScaffold();
	
	Geom::AABB aabb = gManager->scaffold->computeAABB();
	mBox = new BBox(aabb.center(), (aabb.bbmax-aabb.bbmin)*0.5f);

	setManipulatedFrame(activeFrame);

	updateGL();
	//numEdits = 0;

	// Timing
	globalTimer->restart();
}

void MyDesigner::setActiveObject(GraphManager *gm)
{
	// Delete the original object
	if (gManager)
		emit(objectDiscarded());

	// Setup the new object
	gManager = gm;

	// Change title of scene
	setWindowTitle(gManager->scaffold->path);

	// Set camera
	resetView();

	// Update the object
	updateActiveObject();

	//SaveUndo();

	emit(objectInserted());
}

void MyDesigner::newScene()
{
	clearButtons();
	if(gManager)
		emit( objectDiscarded());

	selection.clear();
	
	gManager = NULL;
	setWindowTitle(" ");
	// Update the object
	updateActiveObject();

	//SaveUndo();
	selectMode = SELECT_NONE;
    //segPlane.clear();
	//updateWidget();
	updateGL();
}

void MyDesigner::updateWidget()
{
	/*if(isSceneEmpty){
		designWidget->treeWidget->clear();
		return;
	}
	designWidget->treeWidget->clear();
	int i = 0;
	foreach(HCCGraph::Link* l, mHccGraph->links)
	{
		QTreeWidgetItem *linkItem = new QTreeWidgetItem(designWidget->treeWidget);

		linkItem->setText(0, "Link"+QString::number(i));
		QTreeWidgetItem *item1 = new QTreeWidgetItem(linkItem);
		item1->setText(0, l->node1->mID);
		QTreeWidgetItem *item2 = new QTreeWidgetItem(linkItem);
		item2->setText(0, l->node2->mID);
		i++;
	}*/
}

void MyDesigner::mousePressEvent( QMouseEvent* e )
{
	QGLViewer::mousePressEvent(e);

	if((e->button() == Qt::LeftButton))//(e->modifiers() & Qt::ControlModifier) && 
	{
		this->startMousePos2D = e->pos();
		camera()->convertClickToLine(e->pos(), startMouseOrigin, startMouseDir);

		if(transformMode == SPLIT_MODE)
		{
			/*Line ln(Vec3d(startMouseOrigin[0], startMouseOrigin[1],startMouseOrigin[2]), 
			Vec3d(startMouseDir[0], startMouseDir[1], startMouseDir[2]), startMouseDir.norm());
			*/
			//std::vector<Line> edges = ((Cuboid *)ctrl()->getSelectedPrimitive())->getEdges();
			//Line intLine;
			//Vec3d vec;
			//double minDist = 100000, dist;
			//foreach(Line l, edges){
			//	dist = l.distanceToUnbounded(Vec3d((startMouseOrigin+startMouseDir)[0], (startMouseOrigin+startMouseDir)[1],(startMouseOrigin+startMouseDir)[2]));
			//	if(minDist > dist){
			//		minDist = dist;
			//		intLine = l;
			//	}
	
			//	double cos = (dot(Vec3d(startMouseDir[0],startMouseDir[1],startMouseDir[2]),(intLine.b - intLine.a)))/dot((intLine.b - intLine.a),(intLine.b - intLine.a));
			//	//cos = (cos>0)?cos:1+cos;
			//	vec = fabs(cos) * (intLine.b - intLine.a);
			//	//ln.intersectLine(intLine,interPa, interPb);
			//}
			/*Point p1, p2;
			HCCGraph::Node *n = mHccGraph->getNode(ctrl()->getSelectedPrimitive()->id);  
			n->IntersectRayBox(Point(startMouseOrigin[0],startMouseOrigin[1],startMouseOrigin[2]), 
			Vec3d(startMouseDir[0],startMouseDir[1],startMouseDir[2]), 
			intPnt, p1, p2, vec);
			intLine = Line(p1,p2);
			std::vector<Vector3> face =  ((Cuboid *)ctrl()->getSelectedPrimitive())->getOrthoFace(intLine);
			segPlane.clear();
			foreach(Vector3 p, face)
			segPlane.push_back(p+vec);

			updateHCC(vec);
			updateWidget();*/
		}

		switch(selectMode)
		{
		case BOX: this->setCursor(QCursor(QPixmap(":/Resources/push.png"), 0, 32)); break;
		default: this->setCursor(Qt::ArrowCursor); break;
		}

		if(selectMode == BOX && mBox)
		{
			Point start(startMouseOrigin[0], startMouseOrigin[1],startMouseOrigin[2]);
			Vec3d dir(startMouseDir[0], startMouseDir[1], startMouseDir[2]);
			mBox->selectFace(start,dir);
			if(mBox->axisID < 0){
				selectMode = BOX;
				this->displayMessage("No face has been selected. Left click to select a face to push in");
			}
			this->displayMessage("* Please CTRL + SCROLL THE MOUSE to squeeze the box *", 5000);
		    updateGL();
		}
	}

	if(!isMousePressed)
	{
		if(!defCtrl) return;

		// Set constraints
		if(transformMode == TRANSLATE_MODE)
		{

		}

		if(transformMode == ROTATE_MODE)
		{
			AxisPlaneConstraint * r = new AxisPlaneConstraint;
			r->setTranslationConstraintType(AxisPlaneConstraint::FORBIDDEN);
			defCtrl->getFrame()->setConstraint(r);
		}

		if(transformMode == SCALE_MODE)
		{
			// Forbid everything
			AxisPlaneConstraint * c = new AxisPlaneConstraint;
			c->setRotationConstraintType(AxisPlaneConstraint::FORBIDDEN);
			c->setTranslationConstraintType(AxisPlaneConstraint::FORBIDDEN);
			defCtrl->getFrame()->setConstraint(c);

			Vec3d o(currMouseOrigin[0],currMouseOrigin[1],currMouseOrigin[2]);
			Vec3d r(currMouseDir[0],currMouseDir[1],currMouseDir[2]);

			Vec3d px = rayMeshIntersect(o, r, planeX(defCtrl->pos(), skyRadius));
			Vec3d py = rayMeshIntersect(o, r, planeY(defCtrl->pos(), skyRadius));
			Vec3d pz = rayMeshIntersect(o, r, planeZ(defCtrl->pos(), skyRadius));

			debugPoints.clear();

			Point p(0,0,0);

			// If we got a hit
			if(px.x() < DBL_MAX) p = px;
			if(py.y() < DBL_MAX) p = py;
			if(pz.z() < DBL_MAX) p = pz;

			startScalePos = p;
		}

		if(selectMode == BOX)
		{

		}

		if(transformMode != NONE_MODE && selectMode != SELECT_NONE)
		{
			//SaveUndo();
		}
	}

	isMousePressed = true;
}


void MyDesigner::mouseReleaseEvent( QMouseEvent* e )
{
	QGLViewer::mouseReleaseEvent(e);

	isMousePressed = false;

	if(transformMode == ROTATE_MODE)
	{
		//defCtrl->saveOriginal();
		defCtrl->getFrame()->setOrientation(qglviewer::Quaternion());
	}

	if(transformMode == TRANSLATE_MODE)
	{
		//defCtrl->saveOriginal();
	}

	if(transformMode == SCALE_MODE)
	{
		scaleDelta = Vec3d(1.0);
	}

	if(transformMode == SPLIT_MODE)
	{

	}

	// View changer box area
	double scale = 90;
	int x = e->pos().x(), y = e->pos().y();
	if(x > width() - scale && y > height() - scale && gManager->scaffold)
	{
		QPoint p(abs(x - width() + scale), abs(y - height() + scale));
		Geom::AABB aabb = gManager->scaffold->computeAABB();

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

	//if(isMousePressed && mBox->selPlaneID >= 0){//(e->modifiers() & Qt::ControlModifier) && 
	//	if(selectMode == BOX)
	//	{
	//		camera()->convertClickToLine(e->pos(), currMouseOrigin, currMouseDir);
	//		Point currPos(startMouseOrigin[0], startMouseOrigin[1],startMouseOrigin[2]);
	//		Vec3d currDir(startMouseDir[0], startMouseDir[1], startMouseDir[2]);
	//		Point currPnt;
	//		if(mBox->IntersectRayBox(currPos,currDir,currPnt)){
	//			Point startPnt = mBox->getSelectedFace().Center;
	//			//double factor = currPnt[mBox->axisID] - startPnt[mBox->axisID];
	//			double factor = (currPnt - startPnt).norm() * (currPnt[mBox->axisID] - startPnt[mBox->axisID])/fabs(currPnt[mBox->axisID] - startPnt[mBox->axisID]);
	//			mBox->deform(factor);
	//			mBox->getBoxFaces();
	//		}
	//		else
 //               this->displayMessage("* Fail to push in the right direction *", 5000);
	//		 updateGL();
	//	}
	//}

	if(isMousePressed && defCtrl && !(e->modifiers() & Qt::ShiftModifier))
	{
		Vec3d o(currMouseOrigin[0],currMouseOrigin[1],currMouseOrigin[2]);
		Vec3d r(currMouseDir[0],currMouseDir[1],currMouseDir[2]);

		Vec3d px = rayMeshIntersect(o, r, planeX(defCtrl->pos(), skyRadius));
		Vec3d py = rayMeshIntersect(o, r, planeY(defCtrl->pos(), skyRadius));
		Vec3d pz = rayMeshIntersect(o, r, planeZ(defCtrl->pos(), skyRadius));

		debugPoints.clear();

		Point p(0,0,0);

		// If we got a hit
		if(px.x() < DBL_MAX) p = px;
		if(py.y() < DBL_MAX) p = py;
		if(pz.z() < DBL_MAX) p = pz;

		// Used later
		Vec3d x = Vec3d(1, 0, 0);
		Vec3d y = Vec3d(0, 1, 0);
		Vec3d z = Vec3d(0, 0, 1);

		if(transformMode == SCALE_MODE)
		{
			currScalePos = p;

			//Primitive * prim = ctrl()->getSelectedPrimitive(); if(!prim) return;
		
			Vec3d delta = (currScalePos - startScalePos);

			delta.x() = abs(delta.x());
			delta.y() = abs(delta.y());
			delta.z() = abs(delta.z());
		
			// Project to main axes
			QMultiMap<double,int> proj;
			proj.insert(dot(delta, x), 0);
			proj.insert(dot(delta, y), 1);
			proj.insert(dot(delta, z), 2);

			// Extract sorted values 
			QVector<double> value;
			QVector<int> index;		
			foreach(double key, proj.keys()){
				foreach(int id, proj.values(key)){
					value.push_back(key);
					index.push_back(id);
				}
			}

			// Scaling
			delta = Vec3d(0, 0, 0);
			if (value[2] > value[1] * 3)
			{
				// line
				delta[index[2]] = value[2];
			}
			else if (value[2] < value[0] * 3)
			{
				// cube
				delta[index[0]] = value[0];
				delta[index[1]] = value[1];
				delta[index[2]] = value[2];
			}
			else 
			{
				// plane
				delta[index[1]] = value[1];
				delta[index[2]] = value[2];
			}

			bool isExpand = false;
			Vec3d start = defCtrl->pos();
			if((currScalePos - start).norm() > (startScalePos - start).norm())
				isExpand = true;

			if(isExpand) delta = Vec3d(1.0) + delta;
			else delta = Vec3d(1.0) - delta;

			//defCtrl->scale(delta);

			scaleDelta = delta;

			updateGL();
		}

		if(transformMode == TRANSLATE_MODE)
		{
			// Should constraint to closest axis line..
		}
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
			mBox->deform(factor);
			mBox->getBoxFaces();
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

	//if(e->key() == Qt::Key_Space)	selectPrimitiveMode();
	if(e->key() == Qt::Key_C)		selectCameraMode();
	if(e->key() == Qt::Key_M)		moveMode();
	if(e->key() == Qt::Key_R)		rotateMode();
	if(e->key() == Qt::Key_E)		scaleMode();
	if(e->key() == Qt::Key_A)		pushAABB();

	updateGL();

	if(e->key() != Qt::Key_Space) // disable fly mode..
		QGLViewer::keyPressEvent(e);
}

void MyDesigner::saveObject()
{
	if(gManager && gManager->scaffold)
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
	/*if(activeDeformer) activeDeformer->drawNames();
	if(activeVoxelDeformer) activeVoxelDeformer->drawNames();

	if(!ctrl()) return;

	if(selectMode == CONTROLLER) ctrl()->drawNames(false);
	if(selectMode == CONTROLLER_ELEMENT) ctrl()->drawNames(true);*/
}

void MyDesigner::postSelection( const QPoint& point )
{
	if(currMousePos2D.x() > width() - 90 && currMousePos2D.y() > height() - 90)
		return;

	//int selected = selectedName();

	//// General selection
	//if(selected == -1)
	//	selection.clear();
	//else
	//{
	//	if(selection.contains( selected ))
	//		selection.remove(selection.indexOf(selected));
	//	else
	//	{
	//		if(selectMode != CONTROLLER_ELEMENT)
	//		{
	//			selection.clear();
	//		    ctrl()->selectPrimitive(-1);
	//			selection.push_back(selected); // to start from 0
	//		}
	//	}
	//}

	// Selection mode cases
	switch (selectMode)
	{
	case BOX:
		transformAABB();
		break;
		/*case CONTROLLER:
		transformPrimitive();
		break;

		case CONTROLLER_ELEMENT:
		transformCurve();
		break;

		case EDGE:
		splitCuboid();
		break;*/
	}

	updateGL();
}

void MyDesigner::transformAABB(bool modifySelect)
{
	if(modifySelect)
	{
		transformMode = NONE_MODE;
		designWidget->pushButton->setChecked(true);
		return;
	}

	if(mBox && mBox->selPlaneID >= 0)
	{
		defCtrl = new QManualDeformer(mBox);
		setManipulatedFrame( defCtrl->getFrame() );

		Vec3d q = mBox->getSelectedFace().Center;
		manipulatedFrame()->setPosition( Vec(q.x(), q.y(), q.z()) );
		
		this->connect(defCtrl, SIGNAL(objectModified()), SLOT(updateActiveObject()));
	}
}

void MyDesigner::transformNode(bool modifySelect)
{

}

/*void MyDesigner::transformPrimitive(bool modifySelect)
{
	Controller * c = ctrl();

	if(modifySelect)
	{
		int selected = selectedName();
		c->selectPrimitive(selected);
		if(selected == -1) 
		{
			transformMode = NONE_MODE;
			designWidget->selectCuboidButton->setChecked(true);
		}
		
		return;
	}

	if(!selection.isEmpty())
	{
		if(c->getSelectedPrimitive())
		{
			defCtrl = new QManualDeformer(c);
			setManipulatedFrame( defCtrl->getFrame() );

			Vec3d q = c->getSelectedPrimitive()->centerPoint();
			manipulatedFrame()->setPosition( Vec(q.x(), q.y(), q.z()) );

			this->connect(defCtrl, SIGNAL(objectModified()), SLOT(updateActiveObject()));
		}
	}
}

void MyDesigner::transformCurve(bool modifySelect)
{
	Controller * c = ctrl();
	
	if(!c || !c->getSelectedPrimitive())
		return;

	if(!selection.isEmpty())
	{
		int selected = selectedName();

		if(modifySelect){
			c->selectPrimitiveCurve(selected);
			return;
		}
	
		defCtrl = new QManualDeformer(c);
		setManipulatedFrame( defCtrl->getFrame() );

		Vec3d q = c->getSelectedCurveCenter();
		manipulatedFrame()->setPosition( Vec(q.x(), q.y(), q.z()) );

		this->connect(defCtrl, SIGNAL(objectModified()), SLOT(updateActiveObject()));
	}
}*/
//
//void MyDesigner::splitCuboid(bool modifySelect)
//{
//	Controller * c = ctrl();
//
//	if(!c || !c->getSelectedPrimitive())
//		return;
//
//	if(modifySelect)
//	{
//		int selected = selectedName();
//		c->selectPrimitive(selected);
//		if(selected == -1) 
//		{
//			transformMode = SPLIT_MODE;
//			designWidget->splitButton->setChecked(true);
//		}
//
//		return;
//	}
//}

void MyDesigner::setSelectMode( SelectMode toMode )
{
	this->selectMode = toMode;
}

//Controller * MyDesigner::ctrl()
//{
//	if(!activeObject()) return NULL;
//	return (Controller *) activeObject()->ptr["controller"];
//}
//
//void MyDesigner::selectPrimitiveMode()
//{
//	clearButtons();
//	setManipulatedFrame(activeFrame);
//
//	if(activeMesh == NULL) return;
//
//	setMouseBinding(Qt::LeftButton, SELECT);
//	setMouseBinding(Qt::ShiftModifier | Qt::LeftButton, CAMERA, ROTATE);
//
//	// clear existing selection of curves
//	Controller * c = ctrl();
//	if(c && c->getSelectedPrimitive()) c->getSelectedPrimitive()->selectedCurveId = -1;
//
//	// clear selection of primitives
//	selection.clear();
//	c->selectPrimitive(-1);
//
//	setSelectMode(CONTROLLER);
//	selectTool();
//}

//void MyDesigner::selectCurveMode()
//{
//	clearButtons();
//	setManipulatedFrame(activeFrame);
//
//	if(activeMesh == NULL) return;
//
//	if(ctrl()->getSelectedPrimitive() == NULL)
//	{
//		selectMode = CONTROLLER;
//		selectTool();
//		displayMessage("Select a part first!");
//		return;
//	}
//
//	setMouseBinding(Qt::LeftButton, SELECT);
//	setMouseBinding(Qt::ShiftModifier | Qt::LeftButton, CAMERA, ROTATE);
//
//	setSelectMode(CONTROLLER_ELEMENT);
//	selectTool();
//	
//}
//
//void MyDesigner::selectStackingMode()
//{
//	clearButtons();
//
//	if(activeMesh == NULL) return;
//
//	//designWidget->selectStackingButton->setChecked(true);
//
//	setMouseBinding(Qt::LeftButton, FRAME, ROTATE);
//	setMouseBinding(Qt::ShiftModifier | Qt::LeftButton, CAMERA, ROTATE);
//
//	selection.clear();
//	ctrl()->selectPrimitive(-1);
//
//	selectMode = STACK_DIR_MODE;
//	transformMode = NONE_MODE;
//
//	this->displayMessage("You can now rotate the stacking direction", 1000);
//
//	isDrawStacking = true;
//	//designWidget->showStacking->setChecked(true);
//
//	setManipulatedFrame(&stackingDir);
//}

void MyDesigner::selectCameraMode()
{
	clearButtons();
	setManipulatedFrame(activeFrame);

	if(gManager == NULL) return;

	designWidget->selectCameraButton->setChecked(true);
	
	setMouseBinding(Qt::ShiftModifier | Qt::LeftButton, SELECT);
	setMouseBinding(Qt::LeftButton, CAMERA, ROTATE);

	selection.clear();
	//ctrl()->selectPrimitive(-1);

	selectMode = SELECT_NONE;
	transformMode = NONE_MODE;

	this->displayMessage("Freely move camera", 1000);

	//isDrawStacking = false;
	//designWidget->showStacking->setChecked(isDrawStacking);
}

void MyDesigner::selectTool()
{
	if(gManager == NULL) return;

	if(selectMode == BOX) designWidget->pushButton->setChecked(true);
	//if(selectMode == CONTROLLER) designWidget->selectCuboidButton->setChecked(true);
	//if(selectMode == CONTROLLER_ELEMENT) designWidget->selectCurveButton->setChecked(true);
	transformMode= NONE_MODE;
	updateGL();

	//designWidget->showStacking->setChecked(isDrawStacking);
}

void MyDesigner::pushAABB()
{
	if(!mBox){
		this->displayMessage("* Please import an object first *", 5000);
		return;
	}
	clearButtons();

	/*setMouseBinding(Qt::LeftButton, FRAME, TRANSLATE);
	setMouseBinding(Qt::RightButton, CAMERA, TRANSLATE);*/
	setMouseBinding(Qt::LeftButton, FRAME, TRANSLATE);
	setMouseBinding(Qt::RightButton, CAMERA, TRANSLATE);
	selectMode = BOX;
	toolMode();

	this->displayMessage("* Left click to select a face to push in *", 5000);
}

void MyDesigner::moveMode()
{
	clearButtons();

	setMouseBinding(Qt::LeftButton, FRAME, TRANSLATE);
	setMouseBinding(Qt::RightButton, CAMERA, TRANSLATE);
	transformMode = TRANSLATE_MODE;
	toolMode();
}

void MyDesigner::rotateMode()
{
	clearButtons();

	setMouseBinding(Qt::LeftButton, FRAME, ROTATE);
	setMouseBinding(Qt::ControlModifier | Qt::LeftButton, FRAME, SCREEN_ROTATE);

	transformMode = ROTATE_MODE;
	toolMode();
}

void MyDesigner::scaleMode()
{
	clearButtons();

	//setMouseBinding(Qt::LeftButton, FRAME, NO_MOUSE_ACTION);
    setMouseBinding(Qt::LeftButton, FRAME, TRANSLATE);
	setMouseBinding(Qt::RightButton, CAMERA, TRANSLATE);
	transformMode = SCALE_MODE;
	scaleDelta = Vec3d(1.0);
	toolMode();

	//this->displayMessage("* Please SCROLL THE MOUSE to scale *", 5000);
}

void MyDesigner::splitingMode()
{
	clearButtons();
	setManipulatedFrame(activeFrame);

	if(gManager == NULL) return;

	if(gManager->scaffold== NULL)
	{
		//selectMode = CONTROLLER;
		selectTool();
		displayMessage("Select a splitting plane first!");
		return;
	}

	setMouseBinding(Qt::LeftButton, SELECT);
	setMouseBinding(Qt::ShiftModifier | Qt::LeftButton, CAMERA, ROTATE);

	//setSelectMode(EDGE);
	transformMode = SPLIT_MODE;
	designWidget->splitButton->setChecked(true);
	//selectTool();
	updateGL();
}


void MyDesigner::toolMode()
{
	if(selection.empty() && !mBox && !gManager)
	{
		this->displayMessage("* Please select a part to transform *");

		transformMode = NONE_MODE;
		selectMode = SELECT_NONE;
		
		setMouseBinding(Qt::LeftButton, CAMERA, ROTATE);
	}
	else
	{
		setMouseBinding(Qt::ShiftModifier | Qt::LeftButton, CAMERA, ROTATE);

		// Deal with buttons
		if(transformMode == TRANSLATE_MODE) designWidget->moveButton->setChecked(true);
		if(transformMode == ROTATE_MODE) designWidget->rotateButton->setChecked(true);
		if(transformMode == SCALE_MODE) designWidget->scaleButton->setChecked(true);
		if(transformMode == SPLIT_MODE) designWidget->splitButton->setChecked(true);
		// Connect to QManualDeformer
		//if(selectMode == CONTROLLER) transformPrimitive(false);
		//if(selectMode == CONTROLLER_ELEMENT) transformCurve(false);
		//if(transformMode == SPLIT_MODE) splitCuboid(false);
		if(selectMode == BOX){
			transformAABB(false);
			designWidget->pushButton->setChecked(true);
		}

		this->displayMessage("Press shift to move camera");

		if(transformMode == SCALE_MODE)
			this->displayMessage("Use your mouse wheel to scale a curve", 3000);
	}

	updateGL();
}

void MyDesigner::clearButtons()
{
	designWidget->selectCameraButton->setChecked(false);
	designWidget->selectCuboidButton->setChecked(false);
	designWidget->moveButton->setChecked(false);
	designWidget->rotateButton->setChecked(false);
	designWidget->scaleButton->setChecked(false);
	designWidget->splitButton->setChecked(false);
	designWidget->undoButton->setChecked(false);
	designWidget->redoButton->setChecked(false);
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

int MyDesigner::editTime()
{
	return globalTimer->elapsed();
}

void MyDesigner::showCuboids(int state)
{
	gManager->scaffold->showCuboids(state);
	updateGL();
}
void MyDesigner::showGraph(int state)
{
	gManager->scaffold->showScaffold(state);
	updateGL();
}

void MyDesigner::showModel(int state)
{
	gManager->scaffold->showMeshes(state);
	isShow = (state == Qt::Checked);
	updateGL();
}
