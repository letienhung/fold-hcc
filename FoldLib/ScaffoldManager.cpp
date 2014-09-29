#include "ScaffoldManager.h"
#include "SegMeshLoader.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "Scaffold.h"

#include <QFileInfo>
#include <QFileDialog>


ScaffoldManager::ScaffoldManager()
{
	entireMesh = NULL;
	scaffold = new Scaffold();

	fitMethod = FIT_AABB;
	refitMethod = FIT_MIN;
}


ScaffoldManager::~ScaffoldManager()
{
	delete scaffold;
}


void ScaffoldManager::createScaffold()
{
	SegMeshLoader sml(entireMesh);
	QVector<SurfaceMeshModel*> subMeshes = sml.getSegMeshes();

	// reset scaffold
	scaffold->path = entireMesh->path;
	scaffold->clear();

	// create nodes from meshes
	foreach (SurfaceMeshModel* m, subMeshes)
	{
		scaffold->addNode(MeshPtr(m), fitMethod);
	}

	emit(scaffoldChanged(scaffold));
}

void ScaffoldManager::saveScaffold()
{
	if (!scaffold) return;

	QString dir = QFileInfo(scaffold->path).absolutePath();
	QFileInfo fileInfo(scaffold->path);
	QString filename = QFileDialog::getSaveFileName(0, tr("Save Scaffold"), dir, tr("Graph Files (*.xml)"));
	scaffold->saveToFile(filename);

	emit(message("Saved."));
}

void ScaffoldManager::loadScaffold()
{
	QString dataPath = getcwd() + "/data-syn";
	QString filename = QFileDialog::getOpenFileName(0, tr("Load Scaffold"), dataPath, tr("Graph Files (*.xml)"));
	if (filename.isEmpty()) return;
	scaffold->loadFromFile(filename);

	emit(scaffoldChanged(scaffold));
	emit(message("Loaded."));
}

void ScaffoldManager::setMesh( SurfaceMeshModel* mesh )
{
	this->entireMesh = mesh;
	qDebug() << "Set active mesh as " << entireMesh->path;
}

void ScaffoldManager::changeNodeType()
{
	foreach(Structure::Node* n, scaffold->getSelectedNodes())
	{
		scaffold->changeNodeType((ScaffoldNode*)n);
	}

	emit(scaffoldModified());
}

void ScaffoldManager::refitNodes()
{
	foreach(Structure::Node* n, scaffold->getSelectedNodes())
	{
		ScaffoldNode* fn = (ScaffoldNode*)n;
		fn->refit(refitMethod); 
	}

	emit(scaffoldModified());
}

void ScaffoldManager::linkNodes()
{
	QVector<Structure::Node*> sn = scaffold->getSelectedNodes();
	if (sn.size() < 2) return;

	scaffold->addLink((ScaffoldNode*)sn[0], (ScaffoldNode*)sn[1]);

	emit(scaffoldModified());
}

void ScaffoldManager::test()
{

}

void ScaffoldManager::setFitMethod( int method )
{
	if (method == 0)
		fitMethod = FIT_AABB;
	else if (method == 1)
		fitMethod = FIT_MIN;
	else
		fitMethod = FIT_PCA;
}

void ScaffoldManager::setRefitMethod( int method )
{
	if (method == 0)
		fitMethod = FIT_MIN;
	else if (method == 1)
		fitMethod = FIT_AABB;
	else
		fitMethod = FIT_PCA;
}



