#include "DependGraph.h"
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <QColor>
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include "UtilityGlobal.h"
#include <QQueue>
#include "Numeric.h"


ChainNode::ChainNode( int idx, QString id )
	: Node(id), chainIdx(idx)
{
}

ChainNode::ChainNode( ChainNode &other )
	: Node(other)
{
	chainIdx = other.chainIdx;
}

Structure::Node* ChainNode::clone()
{
	return new ChainNode(*this);
}

FoldingNode::FoldingNode( int hidx, FD_DIRECTION d, QString id )
	: Node(id), hingeIdx(hidx), direct(d), score(0)
{
}

FoldingNode::FoldingNode( FoldingNode &other )
	: Node(other)
{
	hingeIdx = other.hingeIdx;
	direct = other.direct;
	score = other.score;
}

Structure::Node* FoldingNode::clone()
{
	return new FoldingNode(*this);
}


QString DependGraph::dotPath = "\"" + getcwd() + "/FoldLib/GraphVis/dot.exe" + "\"";


DependGraph::DependGraph( QString id )
	:Graph(id)
{
}

DependGraph::DependGraph( DependGraph& other )
	:Graph(other)
{
}

DependGraph::~DependGraph()
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


bool DependGraph::verifyNodeType( QString nid, QString type )
{
	Structure::Node* node = getNode(nid);
	return (node && node->properties["type"] == type);
}


ChainNode* DependGraph::getChainNode( QString fnid )
{
	if (!verifyNodeType(fnid, "folding")) return NULL;

	Structure::Link* l = getFoldinglinks(fnid)[0];
	return (ChainNode*)l->getNodeOther(fnid);
}

QVector<FoldingNode*> DependGraph::getFoldingNodes( QString cnid )
{
	QVector<FoldingNode*> fns;
	if (!verifyNodeType(cnid, "chain")) return fns;

	foreach(Structure::Link* l, getFoldinglinks(cnid))
	{
		fns.push_back((FoldingNode*) l->getNodeOther(cnid));
	}

	return fns;
}


QVector<FoldingNode*> DependGraph::getSiblingFoldingNodes( QString fnid )
{
	if (verifyNodeType(fnid, "chain")) 
		return getFoldingNodes(getChainNode(fnid)->mID);
	else
		return QVector<FoldingNode*>();
}


QVector<FoldingNode*> DependGraph::getAllFoldingNodes()
{
	QVector<FoldingNode*> fns;
	foreach(Structure::Node* n, nodes)
	{
		if (n->properties["type"].toString() == "folding")
			fns << (FoldingNode*)n;
	}

	return fns;
}

QVector<ChainNode*> DependGraph::getAllChainNodes()
{
	QVector<ChainNode*> cns;
	foreach(Structure::Node* n, nodes)
	{
		if (n->properties["type"].toString() == "chain")
			cns << (ChainNode*)n;
	}

	return cns;
}

QVector<Structure::Link*> DependGraph::getFoldinglinks( QString nid )
{
	QVector<Structure::Link*> flinks;
	foreach (Structure::Link* l, getLinks(nid))
	{
		if (l->properties["type"].toString() == "folding")
		{
			flinks.push_back(l);
		}
	}

	return flinks;
}

QVector<Structure::Link*> DependGraph::getCollisionLinks( QString nid )
{
	QVector<Structure::Link*> clinks;
	foreach (Structure::Link* l, getLinks(nid))
	{
		if (l->properties["type"].toString() == "collision")
		{
			clinks.push_back(l);
		}
	}

	return clinks;
}


QString DependGraph::toGraphvizFormat( QString subcaption, QString caption )
{
	QStringList out;
	out << "graph G{\n";
	out << "\t" << "node [ fontcolor = black, color = white];" << "\n";

	// Place on a grid
	double size = 50;
	double spacing = size / 10.0;
	double x = 0, y = 0;
	double dx = size + spacing;
	double dy = dx;
	int length = sqrt((double)nodes.size());

	// Write nodes
	for(int i = 0; i < nodes.size(); i++)
	{
		Structure::Node* node = nodes[i];
		bool isChainNode = node->properties["type"] == "chain";

		QString label = node->mID;
		if (!isChainNode)
		{
			FoldingNode* fnode = (FoldingNode*)node;
			label += "\n" + QString::number(fnode->score);
		}

		QColor color = isChainNode ? Qt::blue : Qt::green;
		QString colorHex; colorHex.sprintf("#%02X%02X%02X", color.red(), color.green(), color.blue());

		QString shape = isChainNode ? "rectangle" : "ellipse";

		out << "\t" << QString("%1 [label = \"%2\", color = \"%3\", shape = %4];").arg(i).arg(label).arg(colorHex).arg(shape) << "\n";

		// Move virtual cursor
		x += dx;
		if(i % length == 0){
			x = 0;
			y += dy;
		}
	}

	// Write links
	for(int i = 0; i < links.size(); i++)
	{
		Structure::Link* link = links[i];

		int n1idx = getNodeIndex(link->nid1);
		int n2idx = getNodeIndex(link->nid2);

		bool isFolding = link->properties["type"] == "folding";
		QString color = isFolding ?  "black" : "red";

		out << "\t\"" << QString::number(n1idx) << "\" -- \"" << QString::number(n2idx) << "\"" 
			<< QString(" [color=\"%1\"]").arg(color) << ";\n";
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

	//QFileInfo info(file);
	//workDir = info.absolutePath();

	out << toGraphvizFormat(subcaption, caption); 

	file.flush();
	file.close();
}

void DependGraph::saveAsImage( QString fname )
{
	// save graphvis
	saveAsGraphviz(fname);

	// convert into png
	QString command = dotPath + QString(" -Tpng %1.gv > %2.png").arg(fname).arg(fname);
	qDebug() << "Executing: " << dotPath << command;
	system(qPrintable(command));
}

// A chain node is free if one of its folding nodes is collision free
bool DependGraph::isFreeChainNode( QString cnid )
{
	Structure::Node* cnode = getNode(cnid);
	if (!cnode || cnode->properties["type"] != "chain")
		return false;

	bool isFree = false;
	foreach(FoldingNode* fnode, getFoldingNodes(cnode->mID))
	{
		if (getCollisionLinks(fnode->mID).isEmpty())
		{
			isFree = true;
			break;
		}
	}

	return isFree;
}


int DependGraph::computeCost( FoldingNode* fnode )
{
	QVector<Structure::Link*> clinks = getCollisionLinks(fnode->mID);

	return clinks.size() * 10;
}

int DependGraph::computeGain( FoldingNode* fnode )
{
	int gain = 0;

	// clone the graph
	DependGraph* g_copy = (DependGraph*)clone();

	// exclude all free chains by themselves
	foreach(ChainNode* cnode, g_copy->getAllChainNodes())
	{
		cnode->properties["visited"] = g_copy->isFreeChainNode(cnode->mID);
	}

	// propagate 
	QQueue<ChainNode*> freeChains;
	ChainNode* cn = g_copy->getChainNode(fnode->mID);
	cn->properties["visited"] = true;
	freeChains.enqueue(cn);

	while(!freeChains.isEmpty())
	{
		ChainNode* cnode = freeChains.dequeue();

		// remove collision links
		foreach (Structure::Link* link, g_copy->getFamilyCollisionLinks(cnode->mID))
		{
			g_copy->removeLink(link);
			gain ++;
		}

		// update free chains 
		foreach(ChainNode* cn, g_copy->getAllChainNodes())
		{
			if (!cn->properties["visited"].toBool() && g_copy->isFreeChainNode(cn->mID))
			{
				cn->properties["visited"] = true;
				freeChains.enqueue(cn);
			}
		}
	}

	delete g_copy;

	return gain;
}

void DependGraph::computeScores()
{
	foreach(FoldingNode* fnode, getAllFoldingNodes())
	{
		double gain = computeGain(fnode);
		double cost = computeCost(fnode);

		fnode->score = gain - cost;
	}
}

FoldingNode* DependGraph::getBestFoldingNode()
{
	FoldingNode* best_fn = NULL;
	double best_score = -maxDouble();
	foreach (FoldingNode* fn, getAllFoldingNodes())
	{
		if (!fn->properties["visited"].toBool())
		{
			if (fn->score > best_score)
			{
				best_score = fn->score;
				best_fn = fn;
			}
		}
	}

	return best_fn;
}

QVector<Structure::Node*> DependGraph::getFamilyNodes( QString nid )
{
	QVector<Structure::Node*> family;

	Structure::Node* node = getNode(nid);
	ChainNode* cnode = (node->properties["type"] == "chain") ? 
						(ChainNode*)node : getChainNode(nid);

	family << cnode;
	foreach(FoldingNode* fn, getFoldingNodes(cnode->mID))
		family << fn;

	return family;
}

QVector<Structure::Link*> DependGraph::getFamilyCollisionLinks( QString nid )
{
	QVector<Structure::Link*> clinks;
	foreach(Structure::Node* node, getFamilyNodes(nid))
	{
		clinks += getCollisionLinks(node->mID);
	}

	return clinks;
}

