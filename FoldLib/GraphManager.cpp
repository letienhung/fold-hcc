#include "GraphManager.h"
#include "SegMeshLoader.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "PointLink.h"
#include "LinearLink.h"
#include "FdGraph.h"

#include <QFileInfo>
#include <QFileDialog>
#include "FdUtility.h"

GraphManager::GraphManager()
{
	this->scaffold = FdGraphPtr(new FdGraph());
	this->entireMesh = NULL;
}


GraphManager::~GraphManager()
{
}


void GraphManager::createScaffold( int method )
{
	SegMeshLoader sml(entireMesh);
	QVector<SurfaceMeshModel*> subMeshes = sml.getSegMeshes();

	// reset scaffold
	scaffold->path = entireMesh->path;
	scaffold->clear();

	// create nodes from meshes
	foreach (SurfaceMeshModel* m, subMeshes)
	{
		scaffold->addNode(m, method);
	}

	emit(scaffoldChanged(scaffold->path));
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


void GraphManager::showAABB( int state )
{
	scaffold->showAABB = (state == Qt::Checked);

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
	if (filename.isEmpty()) return;
	scaffold->loadFromFile(filename);

	emit(scaffoldChanged(scaffold->path));
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

	emit(scaffoldModified());
}

void GraphManager::refitSelectedNodes( int method )
{
	foreach(Structure::Node* n, scaffold->selectedNodes())
	{
		FdNode* fn = (FdNode*)n;
		fn->refit(method); 
	}

	emit(scaffoldModified());
}

void GraphManager::linkSelectedNodes()
{
	QVector<Structure::Node*> sn = scaffold->selectedNodes();
	if (sn.size() < 2) return;

	scaffold->addLink(sn[0], sn[1]);

	emit(scaffoldModified());
}

void GraphManager::test()
{
	scaffold = FdGraphPtr((FdGraph*)(scaffold->clone()));

	emit(scaffoldChanged(scaffold->path));
}

void GraphManager::setScaffold( FdGraphPtr fdg )
{
	scaffold = fdg;
}
