#include "Foldabilizer.h"
#include "FoldabilizerWidget.h"
#include "StarlabDrawArea.h"


QString DEFAULT_FILE_PATH = "..\\..\\data";

#include "Numeric.h"

// test
#include "IntrRectRect.h"

Foldabilizer::Foldabilizer()
{
	widget = NULL;
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

}

void Foldabilizer::updateScene()
{
	drawArea()->updateGL();
}

void Foldabilizer::resetScene()
{
	drawArea()->camera()->showEntireScene();
	drawArea()->updateGL();
}

void Foldabilizer::test()
{

}
Q_EXPORT_PLUGIN(Foldabilizer)
