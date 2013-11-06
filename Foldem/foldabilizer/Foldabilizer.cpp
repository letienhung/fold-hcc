#include "Foldabilizer.h"
#include "foldem_widget.h"
#include "StarlabDrawArea.h"
#include <QFileDialog>


QString DEFAULT_FILE_PATH = "..\\..\\data";

Foldabilizer::Foldabilizer()
{
	widget = NULL;
	hccGraph = new Graph();
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
		hccGraph->computeAABB();
		drawArea()->setSceneBoundingBox(qglviewer::Vec(hccGraph->bbmin), qglviewer::Vec(hccGraph->bbmax));
	}

	drawArea()->camera()->showEntireScene();
	drawArea()->updateGL();
}

void Foldabilizer::createL()
{
	hccGraph->makeL();
	resetScene();
}

void Foldabilizer::createChair()
{
	hccGraph->makeChair();
	resetScene();
}

void Foldabilizer::createT()
{
	hccGraph->makeT();
	resetScene();
}

void Foldabilizer::createX()
{
	hccGraph->makeX();
	resetScene();
}

void Foldabilizer::createU()
{
	hccGraph->makeU();
	resetScene();
}

void Foldabilizer::loadGraph()
{
	QString fileName = QFileDialog::getOpenFileName(0, "Import Mesh", "..\\..\\data", "Mesh Files (*.lcc)"); 

	hccGraph->parseHCC(fileName);
	resetScene();

	DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();
}

void Foldabilizer::jump()
{
	hccGraph->jump();
	resetScene();
}

Q_EXPORT_PLUGIN(Foldabilizer)
