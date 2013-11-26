#include "Foldabilizer.h"
#include "FoldabilizerWidget.h"
#include "StarlabDrawArea.h"
#include <QFileDialog>
#include <QDebug>

QString DEFAULT_FILE_PATH = "..\\..\\data";

// test
#include "IntrRectRect.h"

Foldabilizer::Foldabilizer()
{
	widget = NULL;
	hccGraph = new Graph();
	mhOptimizer = new MHOptimizer(hccGraph);
	stepsPerJump = 1;
}

void Foldabilizer::create()
{
	if (!widget)
	{
        widget = new FoldabilizerWidget(this);

		ModePluginDockWidget *dockwidget = new ModePluginDockWidget("Foldabilizer", mainWindow());
		dockwidget->setWidget(widget);
		mainWindow()->addDockWidget(Qt::RightDockWidgetArea, dockwidget);
	}
}

void Foldabilizer::destroy()
{

}

void Foldabilizer::decorate()
{
	if (hccGraph)
		hccGraph->draw();
}

void Foldabilizer::resetScene()
{
	if (hccGraph){
		drawArea()->setSceneBoundingBox(qglviewer::Vec(hccGraph->bbmin), qglviewer::Vec(hccGraph->bbmax));
	}

	drawArea()->camera()->showEntireScene();
	drawArea()->updateGL();
}

void Foldabilizer::createL()
{
	hccGraph->makeL();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createI()
{
	hccGraph->makeI();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createChair(double legL)
{
	hccGraph->makeChair(legL);
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createT()
{
	hccGraph->makeT();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createX()
{
	hccGraph->makeX();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createSharp()
{
	hccGraph->makeSharp();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createU( double uleft, double umid, double uright )
{
	hccGraph->makeU(uleft, umid, uright);
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createO()
{
	hccGraph->makeO();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::loadGraph()
{
	QString fileName = QFileDialog::getOpenFileName(this->widget, "Import Mesh", DEFAULT_FILE_PATH, "Mesh Files (*.lcc)"); 
	if (fileName.isNull()) return;

	if (hccGraph->loadHCC(fileName))
		resetScene();

	DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();
}

void Foldabilizer::jump()
{
	for( int i = 0; i < stepsPerJump; i++)
	{
		mhOptimizer->jump();
		drawArea()->updateGL();
	}
}

void Foldabilizer::test()
{
	//if (hccGraph->isEmpty()) return;
	//hccGraph->hotAnalyze();
	//drawArea()->updateGL();

	QVector<Vector3> xy;
	xy.push_back(Vector3(1, 0, 0));
	xy.push_back(Vector3(0, 1, 0));

	Vector3 origin(0, 0, 0);
	Vector2 ext0(1, 4), ext1(4, 1);

	Geom::Rectangle r0(origin, xy, ext0); 
	Geom::Rectangle r1(origin, xy, ext1); 

	Geom::IntrRectRect intersector;
	QVector<Vector3> pnts = intersector.test(r0, r1);
}

Q_EXPORT_PLUGIN(Foldabilizer)
