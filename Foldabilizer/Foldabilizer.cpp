#include "Foldabilizer.h"
#include "FoldabilizerWidget.h"
#include "StarlabDrawArea.h"

#include "Graph.h"
#include <QDebug>

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
	using namespace Structure;
	Graph *g = new Graph();
	g->addNode(new Node("node1"));
	g->addNode(new Node("node2"));
	g->addLink(g->getNode("node1"), g->getNode("node2"));

	qDebug() << "I am debugging Graph: "
		<< "nbNodes = " << g->nbNodes() 
		<< "nbLinks = " << g->nbLinks();
}
Q_EXPORT_PLUGIN(Foldabilizer)
