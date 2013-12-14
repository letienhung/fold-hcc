#include "FdPlugin.h"
#include "FdWidget.h"
#include "StarlabDrawArea.h"

#include "Graph.h"
#include <QDebug>

FdPlugin::FdPlugin()
{
	widget = NULL;
	g_manager = new GraphManager();

	this->connect(g_manager, SIGNAL(sceneSettingsChanged()), SLOT(updateScene()));
	this->connect(g_manager, SIGNAL(scaffoldChanged()), SLOT(resetScene()));
	this->connect(g_manager, SIGNAL(message(QString)), SLOT(showStatus(QString)));
}

void FdPlugin::create()
{
	if (!widget)
	{
        widget = new FdWidget(this);

		ModePluginDockWidget *dockwidget = new ModePluginDockWidget("Foldabilizer", mainWindow());
		dockwidget->setWidget(widget);
		mainWindow()->addDockWidget(Qt::RightDockWidgetArea, dockwidget);

		drawArea()->setPerspectiveProjection();
	}

	resetMesh();
}

void FdPlugin::destroy()
{

}

void FdPlugin::decorate()
{
	if (activeScaffold())
	{
		activeScaffold()->draw();
	}
}

void FdPlugin::drawWithNames()
{
	if (activeScaffold())
	{
		activeScaffold()->drawWithNames();
	}
}

void FdPlugin::updateScene()
{
	drawArea()->updateGL();
}

void FdPlugin::resetScene()
{
	Geom::AABB aabb = activeScaffold()->computeAABB();
	qglviewer::Vec bbmin(aabb.bbmin.data());
	qglviewer::Vec bbmax(aabb.bbmax.data());
	drawArea()->camera()->setSceneBoundingBox(bbmin, bbmax);
	drawArea()->camera()->showEntireScene();
	drawArea()->updateGL();
}

void FdPlugin::test()
{
}


FdGraph* FdPlugin::activeScaffold()
{
	return g_manager->scaffold;
}

void FdPlugin::resetMesh()
{
	g_manager->setMesh(mesh());
	showMessage("GraphManager: entireMesh = %s", g_manager->entireMesh->path.toStdString());
}

void FdPlugin::showStatus( QString msg )
{
	showMessage(msg.toStdString().c_str());
}

bool FdPlugin::postSelection( const QPoint& point )
{
	Q_UNUSED(point);

	int nid = drawArea()->selectedName();
	showMessage("Selected Node: id = %d", nid);
	activeScaffold()->selectNode(nid);
	return true;
}


Q_EXPORT_PLUGIN(FdPlugin)
