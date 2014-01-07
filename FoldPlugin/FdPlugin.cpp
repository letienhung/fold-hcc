#include "FdPlugin.h"
#include "FdWidget.h"
#include "StarlabDrawArea.h"

#include "Graph.h"
#include <QDebug>
#include <QProcess>
#include "MeshHelper.h"
#include "PCA.h"

FdPlugin::FdPlugin()
{
	widget = NULL;

	g_manager = new GraphManager();
	this->connect(g_manager, SIGNAL(scaffoldChanged(FdGraph*)), SLOT(resetScene()));
	this->connect(g_manager, SIGNAL(scaffoldModified()), SLOT(updateScene()));
	this->connect(g_manager, SIGNAL(message(QString)), SLOT(showStatus(QString)));

	f_manager = new FoldManager();
	f_manager->connect(g_manager, SIGNAL(scaffoldChanged(FdGraph*)), SLOT(setScaffold(FdGraph*)));
	this->connect(f_manager, SIGNAL(sceneChanged()), SLOT(updateScene()));
	
	drawKeyframe = false;
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
		//Samuel added
		//drawArea()->setSceneRadius(60);
		/*drawArea()->setSceneCenter(qglviewer::Vec(aabb.center().x(), aabb.center().y(), aabb.center().z()));
		drawArea()->camera()->setSceneRadius(30.0);
		drawArea()->camera()->setUpVector(qglviewer::Vec(0,0,1));
		drawArea()->camera()->setPosition(qglviewer::Vec(-10,0,0));
		drawArea()->camera()->lookAt(qglviewer::Vec());*/

		drawArea()->camera()->setSceneBoundingBox(bbmin, bbmax);
		drawArea()->camera()->showEntireScene();
	}
	
	updateScene();
}


void FdPlugin::test()
{
	if(!activeScaffold()) return;

	QVector<Structure::Node*> selNodes = activeScaffold()->getSelectedNodes();
    QVector<QString> nIds;
	if(selNodes.isEmpty()) return;

	foreach(Structure::Node* n, selNodes){
		nIds.push_back(n->mID);
	}

	activeScaffold()->merge(nIds);
	updateScene();
}


FdGraph* FdPlugin::activeScaffold()
{
	if (drawKeyframe) return f_manager->getKeyframe();

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

		FdNode* sn = (FdNode*)activeScaffold()->getNode(nidx);
		if (sn)
		{
			showMessage("Selected name = %d, nodeId = %s, mesh = %s", nidx, qPrintable(sn->mID), qPrintable(sn->mMesh->name));
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

void FdPlugin::showKeyframe( int state )
{
	drawKeyframe = (state == Qt::Checked);
	updateScene();
}


Q_EXPORT_PLUGIN(FdPlugin)
