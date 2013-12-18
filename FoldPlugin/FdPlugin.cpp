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
	this->connect(g_manager, SIGNAL(scaffoldChanged(QString)), SLOT(resetScene()));
	this->connect(g_manager, SIGNAL(scaffoldModified()), SLOT(resetScene()));
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

		// perspective
		drawArea()->setPerspectiveProjection();

		// connections
		widget->connect(g_manager, SIGNAL(scaffoldChanged(QString)), SLOT(setScaffoldName(QString)));
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
	g_manager->test();
}


FdGraphPtr FdPlugin::activeScaffold()
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

	int nidx = drawArea()->selectedName();

	Structure::Node* sn = activeScaffold()->getNode(nidx);
	if (sn)
	{
		showMessage("Selected name = %d, nodeId = %s", nidx, sn->id.toStdString().c_str());
		activeScaffold()->selectNode(nidx);
	}
	else
		showMessage("Selected name = %d", nidx);

	return true;
}

void FdPlugin::fold()
{
	Foldabilizer fdzer(activeScaffold(), Vector3(0, 0, 1));
	fdzer.run();
}


Q_EXPORT_PLUGIN(FdPlugin)
