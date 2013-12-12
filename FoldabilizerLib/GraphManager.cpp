#include "GraphManager.h"
#include "SegMeshLoader.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "FdGraph.h"

#include "PcaObb.h"
#include "AABB.h"
#include "MinOBB.h"

#include <QFileInfo>
#include <QFileDialog>
#include "FdUtility.h"

GraphManager::GraphManager()
{
	this->scaffold = new FdGraph();
	this->entireMesh = NULL;
}

void GraphManager::createScaffold()
{
	SegMeshLoader sml(entireMesh);
	QVector<MeshPtr> subMeshes = sml.getSegMeshes();

	// reset scaffold
	scaffold->path = entireMesh->path;
	scaffold->clear();

	// create nodes from meshes
	foreach (MeshPtr m, subMeshes)
	{
		// create node depends on obb
		Geom::AABB aabb(m.data());
		Geom::Box box = aabb.box();
		int box_type = box.getType(5);

		FdNode* node;
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
	bool show = (state == Qt::Checked);
	foreach(FdNode* n, scaffold->getFdNodes())
		n->showCuboids = show;

	emit(sceneSettingsChanged());
}

void GraphManager::showScaffold( int state )
{
	bool show = (state == Qt::Checked);
	foreach(FdNode* n, scaffold->getFdNodes())
		n->showScaffold = show;

	emit(sceneSettingsChanged());
}


void GraphManager::showMeshes( int state )
{
	bool show = (state == Qt::Checked);
	foreach(FdNode* n, scaffold->getFdNodes())
		n->showMesh = show;

	emit(sceneSettingsChanged());
}


void GraphManager::saveScaffold()
{
	if (!scaffold) return;
	QFileInfo fileInfo(scaffold->path);
	QString filename = QFileDialog::getSaveFileName(0, tr("Save Scaffold"), "../data", tr("Graph Files (*.xml)"));
	scaffold->saveToFile(filename);

	emit(message("Saved."));
}

void GraphManager::loadScaffold()
{
	QString filename = QFileDialog::getOpenFileName(0, tr("Load Scaffold"), "../data", tr("Graph Files (*.xml)"));
	scaffold->loadFromFile(filename);

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
