#include "FdGraph.h"

#include <QFile>
#include <QFileInfo>
#include "XmlWriter.h"

FdGraph::FdGraph()
{
}

QVector<FdNode*> FdGraph::getFdNodes()
{
	QVector<FdNode*> fdns;
	foreach(Structure::Node* n, nodes)
		fdns.push_back((FdNode*)n);
	
	return fdns;
}

void FdGraph::saveToFile( QString fname )
{
	QFile file(fname);
	if (!file.open(QIODevice::WriteOnly)) return;

	XmlWriter xw(&file);
	xw.setAutoNewLine(true);	

	// header
	xw.writeRaw("\<!--?xml Version = \"1.0\"?--\>\n");

	// document
	xw.writeOpenTag("document");
	{
		// nodes
		xw.writeTaggedString("cN", QString::number(nbNodes()));
		foreach(FdNode* node, getFdNodes())
			node->writeToXml(xw);

		// links
		xw.writeTaggedString("cL", QString::number(nbLinks()));
	}
	xw.writeCloseTag("document");
	file.close();
}
