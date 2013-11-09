#include "Foldabilizer.h"
#include "foldem_widget.h"
#include "StarlabDrawArea.h"
#include <QFileDialog>

#include <QProcess>

QString DEFAULT_FILE_PATH = "..\\..\\data";

Foldabilizer::Foldabilizer()
{
	widget = NULL;
	hccGraph = new Graph();
	mhOptimizer = new MHOptimizer(hccGraph);
}

void Foldabilizer::create()
{
	if (!widget)
	{
		widget = new foldem_widget(this);

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
	hccGraph->computeAabb();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createI()
{
	hccGraph->makeI();
	hccGraph->computeAabb();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createChair()
{
	hccGraph->makeChair();
	hccGraph->computeAabb();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createT()
{
	hccGraph->makeT();
	hccGraph->computeAabb();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createX()
{
	hccGraph->makeX();
	hccGraph->computeAabb();
	mhOptimizer->isReady = false;
	resetScene();
}

void Foldabilizer::createU()
{
	hccGraph->makeU();
	hccGraph->computeAabb();
	mhOptimizer->isReady = false;
	resetScene();
}


void Foldabilizer::createO()
{
	hccGraph->makeO();
	hccGraph->computeAabb();
	mhOptimizer->isReady = false;
	resetScene();
}


void Foldabilizer::loadGraph()
{
	QString fileName = QFileDialog::getOpenFileName(0, "Import Mesh", "..\\..\\data", "Mesh Files (*.lcc)"); 

	hccGraph->loadHCC(fileName);
	resetScene();

	DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();
}

void Foldabilizer::jump()
{
	mhOptimizer->jump();
	drawArea()->updateGL();
}

Q_EXPORT_PLUGIN(Foldabilizer)
