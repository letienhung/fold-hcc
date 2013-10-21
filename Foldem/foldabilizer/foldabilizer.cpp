#include "foldabilizer.h"
#include "foldem_widget.h"
#include "StarlabDrawArea.h"

foldabilizer::foldabilizer()
{
	widget = NULL;
}

void foldabilizer::create()
{
	if (!widget)
	{
		widget = new foldem_widget(this);

		ModePluginDockWidget *dockwidget = new ModePluginDockWidget("Foldabilize HCC", mainWindow());
		dockwidget->setWidget(widget);
		mainWindow()->addDockWidget(Qt::RightDockWidgetArea, dockwidget);
	}
}

void foldabilizer::destroy()
{

}

void foldabilizer::decorate()
{

}

Q_EXPORT_PLUGIN(foldabilizer)