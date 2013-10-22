#include "Foldabilizer.h"
#include "foldem_widget.h"
#include "StarlabDrawArea.h"

// Debug
#include "Graph.h"

Foldabilizer::Foldabilizer()
{
	widget = NULL;

	// Debug
	Graph *g = new Graph();
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

}

Q_EXPORT_PLUGIN(Foldabilizer)
