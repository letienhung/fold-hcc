#include "FoldabilizerUi.h"
#include "FuiWidget.h"
#include <QDebug>

FoldabilizerUi::FoldabilizerUi()
{
	widget = NULL;
}

void FoldabilizerUi::create()
{
	if (!widget)
	{
		widget = new FuiWidget(this);

		ModePluginDockWidget *dockwidget = new ModePluginDockWidget("Foldabilizer UI", mainWindow());
		dockwidget->setWidget(widget);
		mainWindow()->addDockWidget(Qt::RightDockWidgetArea, dockwidget);
	}
}

void FoldabilizerUi::destroy()
{

}

void FoldabilizerUi::decorate()
{

}

void FoldabilizerUi::sayHi()
{
	qDebug() << "Hi!";
}


Q_EXPORT_PLUGIN(FoldabilizerUi)