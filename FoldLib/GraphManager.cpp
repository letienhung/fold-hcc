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

	QString dir = QFileInfo(scaffold->path).absolutePath();
	QFileInfo fileInfo(scaffold->path);
	QString filename = QFileDialog::getSaveFileName(0, tr("Save Scaffold"), dir, tr("Graph Files (*.xml)"));
	scaffold->saveToFile(filename);

	emit(message("Saved."));
}

void GraphManager::loadScaffold()
{
	QString dataPath = getcwd() + "/data";
	QString filename = QFileDialog::getOpenFileName(0, tr("Load Scaffold"), dataPath, tr("Graph Files (*.xml)"));
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
		scaffold->changeNodeType((FdNode*)n);
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

	scaffold->addLink((FdNode*)sn[0], (FdNode*)sn[1]);

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



