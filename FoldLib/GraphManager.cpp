#include "GraphManager.h"
#include "SegMeshLoader.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "PointLink.h"
#include "LinearLink.h"
#include "FdGraph.h"

#include <QFileInfo>
#include <QFileDialog>

GraphManager::GraphManager()
{
	entireMesh = NULL;
	scaffold = new FdGraph();

	fitMethod = 0;
	refitMethod = 0;
}


GraphManager::~GraphManager()
{
	delete scaffold;
}


void GraphManager::createScaffold()
{
	SegMeshLoader sml(entireMesh);
	QVector<SurfaceMeshModel*> subMeshes = sml.getSegMeshes();

	// reset scaffold
	scaffold->path = entireMesh->path;
	scaffold->clear();

	// create nodes from meshes
	foreach (SurfaceMeshModel* m, subMeshes)
	{
		scaffold->addNode(m, fitMethod);
	}

	emit(scaffoldChanged(scaffold));
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

	emit(scaffoldChanged(scaffold));
	emit(message("Loaded."));
}

void GraphManager::setMesh( SurfaceMeshModel* mesh )
{
	this->entireMesh = mesh;
	qDebug() << "Set active mesh as " << entireMesh->path;
}

void GraphManager::changeNodeType()
{
	foreach(Structure::Node* n, scaffold->getSelectedNodes())
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

void GraphManager::refitNodes()
{
	foreach(Structure::Node* n, scaffold->getSelectedNodes())
	{
		FdNode* fn = (FdNode*)n;
		fn->refit(refitMethod); 
	}

	emit(scaffoldModified());
}

void GraphManager::linkNodes()
{
	QVector<Structure::Node*> sn = scaffold->getSelectedNodes();
	if (sn.size() < 2) return;

	scaffold->addLink(sn[0], sn[1]);

	emit(scaffoldModified());
}

void GraphManager::test()
{

}

void GraphManager::setFitMethod( int method )
{
	fitMethod = method;
}

void GraphManager::setRefitMethod( int method )
{
	refitMethod = method;
}


