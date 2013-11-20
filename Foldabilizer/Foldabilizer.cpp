#include "Foldabilizer.h"
#include "FoldabilizerWidget.h"
#include "StarlabDrawArea.h"
#include <QFileDialog>
#include <QDebug>


QString DEFAULT_FILE_PATH = "..\\..\\data";

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

void Foldabilizer::createU()
{
	hccGraph->makeU();
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
	if (hccGraph->isEmpty()) return;
	hccGraph->hotAnalyze();
	drawArea()->updateGL();
}

void Foldabilizer::setLinkProbability( double lp )
{
	mhOptimizer->setLinkProbability(lp);
}

void Foldabilizer::setCostWeight( double weight )
{
	mhOptimizer->costWeight = weight;
}

void Foldabilizer::setTemprature( int T )
{
	mhOptimizer->temperature = T;
}

void Foldabilizer::setStepsPerJump( int steps )
{
	this->stepsPerJump = steps;
}

void Foldabilizer::setTargetVolumePercentage(int v)
{
	mhOptimizer->targetVolumePercentage = v / (double)100;
}

Q_EXPORT_PLUGIN(Foldabilizer)
