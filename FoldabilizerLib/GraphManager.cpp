#include "GraphManager.h"
#include "SegMeshLoader.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "FdGraph.h"

#include "PcaObb.h"
#include "AABB.h"
#include "MinOBB.h"

#include <QFileInfo>
#include "FdUtility.h"

GraphManager::GraphManager()
{
	this->scaffold = new FdGraph();
	this->entireMesh = NULL;
}

void GraphManager::createScaffold(bool dofitting)
{
	SegMeshLoader sml(entireMesh);
	QVector<SurfaceMeshModel*> subMeshes = sml.getSegMeshes();

	// reset scaffold
	scaffold->path = entireMesh->path;
	scaffold->clear();

	// create nodes from meshes
	foreach (SurfaceMeshModel* m, subMeshes)
	{
		Geom::Box box;
		// fit bb
		if (dofitting)
		{
			Geom::MinOBB bb(m);
			box = bb.mMinBox;
		}
		else if (boxMap.contains(m->name))
		{
			box = boxMap[m->name];
		}
		else
		{
			emit(message("Loading scaffold failed: box doesn't exist."));
		}

		// create node depends on obb
		FdNode * node;
		int box_type = box.getType(5);
		if (box_type == Geom::Box::ROD)	
			 node = new RodNode(m, box);
		else node = new PatchNode(m, box);

		// add to scaffold
		scaffold->addNode(node);
	}

	emit(scaffoldChanged());
}

void GraphManager::showCuboids( int state )
{
	foreach(FdNode* n, scaffold->getFdNodes())
		n->showCuboids = (state == Qt::Checked);

	emit(sceneSettingsChanged());
}

void GraphManager::showScaffold( int state )
{
	foreach(FdNode* n, scaffold->getFdNodes())
		n->showScaffold = (state == Qt::Checked);

	emit(sceneSettingsChanged());
}

void GraphManager::saveScaffold()
{
	if (!scaffold) return;

	QFileInfo finfo(scaffold->path);
	QString fname = finfo.absolutePath() + '/'+ finfo.baseName() + ".xml";
	scaffold->saveToFile(fname);

	emit(message("Saved."));
}

void GraphManager::loadScaffold()
{
	if (!entireMesh) return;

	// open the file
	QFileInfo finfo(entireMesh->path);
	QString fname = finfo.absolutePath() + '/'+ finfo.baseName() + ".xml";
	QFile file(fname);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	// cast as dom
	QDomDocument doc;
	if (!doc.setContent(&file, false)) {
		file.close();
		return;
	}
	file.close();

	// parse dom
	boxMap.clear();
	QDomNodeList nodeList = doc.firstChildElement("document").elementsByTagName("node");
	for (int i = 0; i < nodeList.size(); i++)
	{
		QDomNode node = nodeList.at(i);
		QString nid = node.firstChildElement("ID").text();
		Geom::Box box = getBox(node.firstChildElement("box"));

		boxMap[nid] = box;
	}

	// create scaffold w\o fitting
	this->createScaffold(false);

	emit(scaffoldChanged());
	emit(message("Loaded."));
}

void GraphManager::setMesh( Model* model )
{
	this->entireMesh = (SurfaceMeshModel*) model;
	qDebug() << "Set active mesh as " << entireMesh->path;
}

void GraphManager::linkSelectedNodes()
{

}
