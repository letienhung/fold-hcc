#include "FdGraph.h"

#include <QFile>
#include <QFileInfo>
#include "xmlWriter.h"

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
		xw.writeOpenTag("nodes");
		{
			xw.writeTaggedString("count", QString::number(nbNodes()));

			foreach(FdNode* node, getFdNodes())
				node->writeToXml(xw);
		}
		xw.writeCloseTag("nodes");

		// links
		xw.writeOpenTag("links");
		{
			xw.writeTaggedString("count", QString::number(nbLinks()));


		}
		xw.writeCloseTag("links");
	}
	xw.writeCloseTag("document");
}
