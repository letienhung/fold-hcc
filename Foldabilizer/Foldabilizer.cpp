#include "Foldabilizer.h"
#include "FoldabilizerWidget.h"
#include "StarlabDrawArea.h"

#include "Graph.h"
#include <QDebug>

Foldabilizer::Foldabilizer()
{
	widget = NULL;
	g_manager = new GraphManager();

	this->connect(g_manager, SIGNAL(sceneSettingsChanged()), SLOT(updateScene()));
	this->connect(g_manager, SIGNAL(scaffoldChanged()), SLOT(resetScene()));
	this->connect(g_manager, SIGNAL(message(QString)), SLOT(showStatus(QString)));
}

void Foldabilizer::create()
{
	if (!widget)
	{
        widget = new FoldabilizerWidget(this);

		ModePluginDockWidget *dockwidget = new ModePluginDockWidget("Foldabilizer", mainWindow());
		dockwidget->setWidget(widget);
		mainWindow()->addDockWidget(Qt::RightDockWidgetArea, dockwidget);

		drawArea()->setPerspectiveProjection();
	}

	resetMesh();
}

void Foldabilizer::destroy()
{

}

void Foldabilizer::decorate()
{
	if (activeScaffold())
	{
		activeScaffold()->draw();
	}
}

void Foldabilizer::drawWithNames()
{
	if (activeScaffold())
	{
		activeScaffold()->drawWithNames();
	}
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
	using namespace Structure;
	Graph *g = new Graph();
	g->addNode(new Node("node1"));
	g->addNode(new Node("node2"));
	g->addLink(g->getNode("node1"), g->getNode("node2"));

	qDebug() << "I am debugging Graph: "
		<< "nbNodes = " << g->nbNodes() 
		<< "nbLinks = " << g->nbLinks();
}


FdGraph* Foldabilizer::activeScaffold()
{
	return g_manager->scaffold;
}

void Foldabilizer::resetMesh()
{
	g_manager->setMesh(mesh());
	showMessage("GraphManager: entireMesh = %s", g_manager->entireMesh->path.toStdString());
}

void Foldabilizer::showStatus( QString msg )
{
	showMessage(msg.toStdString().c_str());
}

bool Foldabilizer::postSelection( const QPoint& point )
{
	int nid = drawArea()->selectedName();
	showMessage("Selected Node: id = %d", nid);
	activeScaffold()->selectNode(nid);
	return true;
}


Q_EXPORT_PLUGIN(Foldabilizer)
