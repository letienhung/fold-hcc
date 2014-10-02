#include "ScaffManager.h"
#include "SegMeshLoader.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "Scaffold.h"

#include <QFileInfo>
#include <QFileDialog>


ScaffManager::ScaffManager()
{
	entireMesh = nullptr;
	scaffold = new Scaffold();

	fitMethod = FIT_AABB;
	refitMethod = FIT_MIN;
}


ScaffManager::~ScaffManager()
{
	delete scaffold;
}


void ScaffManager::createScaffold()
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

void ScaffManager::saveScaffold()
{
	if (!scaffold) return;

	QString dir = QFileInfo(scaffold->path).absolutePath();
	QFileInfo fileInfo(scaffold->path);
	QString filename = QFileDialog::getSaveFileName(0, tr("Save Scaffold"), dir, tr("Graph Files (*.xml)"));
	scaffold->saveToFile(filename);

	emit(message("Saved."));
}

void ScaffManager::loadScaffold()
{
	QString dataPath = getcwd() + "/data-syn";
	QString filename = QFileDialog::getOpenFileName(0, tr("Load Scaffold"), dataPath, tr("Graph Files (*.xml)"));
	if (filename.isEmpty()) return;
	scaffold->loadFromFile(filename);

	emit(scaffoldChanged(scaffold));
	emit(message("Loaded."));
}

void ScaffManager::setMesh( SurfaceMeshModel* mesh )
{
	this->entireMesh = mesh;
	qDebug() << "Set active mesh as " << entireMesh->path;
}

void ScaffManager::changeNodeType()
{
	foreach(Structure::Node* n, scaffold->getSelectedNodes())
	{
		scaffold->changeNodeType((ScaffNode*)n);
	}

	emit(scaffoldModified());
}

void ScaffManager::refitNodes()
{
	foreach(Structure::Node* n, scaffold->getSelectedNodes())
	{
		ScaffNode* fn = (ScaffNode*)n;
		fn->refit(refitMethod); 
	}

	emit(scaffoldModified());
}

void ScaffManager::linkNodes()
{
	QVector<Structure::Node*> sn = scaffold->getSelectedNodes();
	if (sn.size() < 2) return;

	scaffold->addLink((ScaffNode*)sn[0], (ScaffNode*)sn[1]);

	emit(scaffoldModified());
}

void ScaffManager::test()
{

}

void ScaffManager::setFitMethod( int method )
{
	if (method == 0)
		fitMethod = FIT_AABB;
	else if (method == 1)
		fitMethod = FIT_MIN;
	else
		fitMethod = FIT_PCA;
}

void ScaffManager::setRefitMethod( int method )
{
	if (method == 0)
		fitMethod = FIT_MIN;
	else if (method == 1)
		fitMethod = FIT_AABB;
	else
		fitMethod = FIT_PCA;
}



