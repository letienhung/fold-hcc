#include "Foldabilizer.h"
#include "foldem_widget.h"
#include "StarlabDrawArea.h"


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
		drawArea()->setSceneRadius(hccGraph->radius);
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

Q_EXPORT_PLUGIN(Foldabilizer)
