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
	QVector<MeshPtr> subMeshes = sml.getSegMeshes();

	// reset scaffold
	scaffold->path = entireMesh->path;
	scaffold->clear();

	// create nodes from meshes
	foreach (MeshPtr m, subMeshes)
	{
		Geom::Box box;
		// fit bb
		if (dofitting)
		{
			Geom::AABB aabb(m.data());
			box = aabb.box();
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
		FdNode* node;
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

void GraphManager::changeTypeOfSelectedNodes()
{
	foreach(Structure::Node* n, scaffold->selectedNodes())
	{
		// create new node
		FdNode* old_node = (FdNode*)n;
		Structure::Node* new_node;
		if (old_node->mType == FdNode::PATCH)
			new_node =new RodNode(old_node->mMesh, old_node->mBox);
		else
			new_node = new PatchNode(old_node->mMesh, old_node->mBox);

		// replace
		scaffold->replaceNode(old_node, new_node);
	}

	emit(scaffoldChanged());
}

void GraphManager::refitSelectedNodes( int method )
{
	foreach(Structure::Node* n, scaffold->selectedNodes())
	{
		FdNode* fn = (FdNode*)n;
		fn->refit(method);
	}

	emit(scaffoldChanged());
}

void GraphManager::linkSelectedNodes()
{
	QVector<Structure::Node*> snodes = scaffold->selectedNodes();
	if (snodes.size() < 2) return;

	scaffold->addLink(snodes[0], snodes[1]);
}
