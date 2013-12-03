#include "Foldabilizer.h"
#include "FoldabilizerWidget.h"
#include "StarlabDrawArea.h"
#include <QFileDialog>
#include <QDebug>

QString DEFAULT_FILE_PATH = "..\\..\\data";

#include "Numeric.h"

// test
#include "IntrRectRect.h"

Foldabilizer::Foldabilizer()
{
	widget = NULL;
	hccGraph = new Graph();
	mhOptimizer = new MHOptimizer(hccGraph);
	connect(mhOptimizer, SIGNAL(hccChanged()), drawArea(), SLOT(update()));
	stepsPerJump = 1;

	this->connect(this, SIGNAL(hccGraphChanged()), SLOT(resetScene()));
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


void Foldabilizer::updateScene()
{
	drawArea()->updateGL();
}


void Foldabilizer::resetScene()
{
	if (hccGraph)
		drawArea()->setSceneBoundingBox(qglviewer::Vec(hccGraph->bbmin), qglviewer::Vec(hccGraph->bbmax));
	
	if (mhOptimizer)
		mhOptimizer->isReady = false;

	drawArea()->camera()->showEntireScene();
	drawArea()->updateGL();
}


void Foldabilizer::createL()
{
	hccGraph->makeL();
	emit(hccGraphChanged());
}

void Foldabilizer::createI()
{
	hccGraph->makeI();
	emit(hccGraphChanged());
}

void Foldabilizer::createChair(double legL)
{
	hccGraph->makeChair(legL);
	emit(hccGraphChanged());

}

void Foldabilizer::createT()
{
	hccGraph->makeT();
	emit(hccGraphChanged());
}

void Foldabilizer::createX()
{
	hccGraph->makeX();
	emit(hccGraphChanged());
}

void Foldabilizer::createSharp()
{
	hccGraph->makeSharp();
	emit(hccGraphChanged());
}

void Foldabilizer::createU( double uleft, double umid, double uright )
{
	hccGraph->makeU(uleft, umid, uright);	
	emit(hccGraphChanged());
}

void Foldabilizer::createO()
{
	hccGraph->makeO();
	emit(hccGraphChanged());
}

void Foldabilizer::createO_2()
{
	hccGraph->makeO_2();
	emit(hccGraphChanged());
}

void Foldabilizer::createBox()
{
	hccGraph->makeBox();
	emit(hccGraphChanged());
}

void Foldabilizer::loadGraph()
{
	QString fileName = QFileDialog::getOpenFileName(this->widget, "Import Mesh", DEFAULT_FILE_PATH, "Mesh Files (*.lcc)"); 
	if (fileName.isNull()) return;

	if (hccGraph->loadHCC(fileName))
		emit(hccGraphChanged());

	DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();
}

void Foldabilizer::jump()
{
	for( int i = 0; i < stepsPerJump; i++)
	{
		mhOptimizer->jump();
		updateScene();
	}
}

void Foldabilizer::test()
{
	int k = 3;
	int n = 2*k - 1;
	double m1 = 1;
	double eps = m1 / k;
	double m2 = m1 - eps;

	// H
	double L0 = 2*m1 - eps;

	// N
	double N0 = 2*m1 - (m1-m2)/pow(2.0, double(k-2));
	double N1;
	double L1 = Max(N0, N1);
}

Q_EXPORT_PLUGIN(Foldabilizer)
