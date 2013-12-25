#include "DependGraph.h"
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QColor>
#include <QDebug>

DependGraph::DependGraph( QString id )
	:Graph(id)
{
}

DependGraph::DependGraph( DependGraph& other )
	:Graph(other)
{
}

Structure::Graph* DependGraph::clone()
{
	return new DependGraph(*this);
}


void DependGraph::addNode( ChainNode* cn )
{
	Graph::addNode(cn);
	cn->properties["type"] = "chain";
}

void DependGraph::addNode( FoldingNode* fn )
{
	Graph::addNode(fn);
	fn->properties["type"] = "folding";
}


void DependGraph::addFoldingLink( Structure::Node* n1, Structure::Node* n2 )
{
	Structure::Link* link =  Graph::addLink(n1, n2);
	link->properties["type"] = "folding";
}

void DependGraph::addCollisionLink( Structure::Node* n1, Structure::Node* n2 )
{
	Structure::Link* link =  Graph::addLink(n1, n2);
	link->properties["type"] = "collision";
}

ChainNode* DependGraph::getChainNode( QString id )
{
	return (ChainNode*)getNode(id);
}

QVector<FoldingNode*> DependGraph::getFoldingNodes( QString id )
{
	QVector<FoldingNode*> fns;
	foreach (Structure::Node* n, getNeighbourNodes(getNode(id)))
	{
		if (n->properties["type"].toString() == "folding")
			fns << (FoldingNode*)n;
	}

	return fns;
}

QString DependGraph::toGraphvizFormat( QString subcaption, QString caption )
{
	QStringList out;
	out << "graph G{\n";
	out << "\t" << "node [ fontcolor = black, color = white, style = filled ];" << "\n";

	// Place on a grid
	double size = 50;
	double spacing = size / 10.0;
	double x = 0, y = 0;
	double dx = size + spacing;
	double dy = dx;
	int length = sqrt((double)nodes.size());

	// Write nodes
	for(int i; i < nodes.size(); i++)
	{
		Structure::Node* node = nodes[i];
		bool isChainNode = node->properties["type"] == "chain";

		QColor color = isChainNode ? Qt::blue : Qt::green;
		QString colorHex; colorHex.sprintf("#%02X%02X%02X", color.red(), color.green(), color.blue());

		QString shape = isChainNode ? "rectangle" : "ellipse";

		out << "\t" << QString("%1 [label = \"%2\", color = \"%3\", shape = %4];").arg(i).arg( node->mID ).arg(colorHex).arg(shape) << "\n";

		// Move virtual cursor
		x += dx;
		if(i % length == 0){
			x = 0;
			y += dy;
		}
	}

	// Write links
	for(int i; i < links.size(); i++)
	{
		Structure::Link* link = links[i];

		int n1idx = getNodeIndex(link->nid1);
		int n2idx = getNodeIndex(link->nid2);

		bool isFolding = link->properties["type"] == "folding";
		QString color = isFolding ?  "black" : "red";

		out << "\t\"" << QString::number(n1idx) << "\" -- \"" << QString::number(n2idx) << "\"" 
			<< QString(" [color=\"%1\"] ").arg(color) << ";\n";
	}

	// Labels
	out << "label = \"\\n\\n" << caption << "\\n" << subcaption << "\"\n";
	out << "fontsize = 20;\n";

	out << "}\n";

	return out.join("");
}

void DependGraph::saveAsGraphviz( QString fname, QString subcaption /*= ""*/, QString caption /*= ""*/ )
{
	QFile file(fname + ".gv");
	if (!file.open(QFile::WriteOnly | QFile::Text))	return;
	QTextStream out(&file);

	out << toGraphvizFormat(subcaption, caption); 

	file.flush();
	file.close();
}

void DependGraph::saveAsImage( QString fname )
{
	saveAsGraphviz(fname);

	// Assuming Graphviz is installed
	QString command = QString("dot %1 -Tpng > %2").arg(fname+".gv").arg(fname+".png");
	qDebug() << "Executing: " << command;
	system(qPrintable(command));
}

