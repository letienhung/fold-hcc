#include "FdPlugin.h"
#include "FdWidget.h"
#include "StarlabDrawArea.h"

#include "Graph.h"
#include <QDebug>

FdPlugin::FdPlugin()
{
	widget = NULL;

	g_manager = new GraphManager();
	this->connect(g_manager, SIGNAL(scaffoldChanged(FdGraph*)), SLOT(resetScene()));
	this->connect(g_manager, SIGNAL(scaffoldModified()), SLOT(updateScene()));
	this->connect(g_manager, SIGNAL(message(QString)), SLOT(showStatus(QString)));

	f_manager = new FoldManager();
	f_manager->connect(g_manager, SIGNAL(scaffoldChanged(FdGraph*)), SLOT(setScaffold(FdGraph*)));
	this->connect(f_manager, SIGNAL(selectionChanged()), SLOT(updateScene()));
	
	drawFolded = false;
	drawAABB = false;
	drawCuboid = true;
	drawScaffold = true;
	drawMesh = false;
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
	}

	resetMesh();
}

void FdPlugin::destroy()
{

}

void FdPlugin::decorate()
{
	if (activeScaffold())
		activeScaffold()->draw();
}

void FdPlugin::drawWithNames()
{
	if (activeScaffold())
		activeScaffold()->drawWithNames();
}

void FdPlugin::updateScene()
{
	if (activeScaffold())
	{
		activeScaffold()->showAABB = drawAABB;
		activeScaffold()->showCuboids(drawCuboid);
		activeScaffold()->showScaffold(drawScaffold);
		activeScaffold()->showMeshes(drawMesh);
	}

	drawArea()->updateGL();
}

void FdPlugin::resetScene()
{
	if (activeScaffold())
	{
		Geom::AABB aabb = activeScaffold()->computeAABB();
		aabb.validate();

		qglviewer::Vec bbmin(aabb.bbmin.data());
		qglviewer::Vec bbmax(aabb.bbmax.data());
		drawArea()->camera()->setSceneBoundingBox(bbmin, bbmax);
		drawArea()->camera()->showEntireScene();
	}
	
	updateScene();
}

void FdPlugin::test()
{

}


FdGraph* FdPlugin::activeScaffold()
{
	return (drawFolded ? f_manager->activeScaffold() : g_manager->scaffold);
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

	if (activeScaffold())
	{
		int nidx = drawArea()->selectedName();

		Structure::Node* sn = activeScaffold()->getNode(nidx);
		if (sn)
		{
			showMessage("Selected name = %d, nodeId = %s", nidx, sn->mID.toStdString().c_str());
			activeScaffold()->selectNode(nidx);
		}
		else
			showMessage("Selected name = %d", nidx);
	}

	return true;
}

void FdPlugin::showFolded( int state )
{
	drawFolded = (state == Qt::Checked);
	updateScene();
}


void FdPlugin::showAABB( int state )
{
	drawAABB = (state == Qt::Checked);
	updateScene();
}

void FdPlugin::showCuboid( int state )
{
	drawCuboid = (state == Qt::Checked);
	updateScene();
}

void FdPlugin::showScaffold( int state )
{
	drawScaffold = (state == Qt::Checked);
	updateScene();
}

void FdPlugin::showMesh( int state )
{
	drawMesh = (state == Qt::Checked);
	updateScene();
}


Q_EXPORT_PLUGIN(FdPlugin)
