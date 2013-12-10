#include "GraphManager.h"
#include "SegMeshLoader.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "FdGraph.h"

#include "PcaObb.h"
#include "AABB.h"
#include "MinOBB.h"

#include <QFileInfo>

GraphManager::GraphManager()
{
	this->scaffold = new FdGraph();
	this->entireMesh = NULL;
}

void GraphManager::createScaffold()
{
	SegMeshLoader sml(entireMesh);
	QVector<SurfaceMeshModel*> subMeshes = sml.getSegMeshes();

	// reset scaffold
	scaffold->path = entireMesh->path;
	scaffold->clear();

	// create nodes from meshes
	Geom::MinOBB bb;
	foreach (SurfaceMeshModel* m, subMeshes)
	{
		// fit bb
		bb.computeMinOBB(m);
		Geom::Box box = bb.mMinBox;

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
}

void GraphManager::loadScaffold()
{
	if (!entireMesh) return;

	QFileInfo finfo(scaffold->path);
	QString fname = finfo.absolutePath() + '/'+ finfo.baseName() + ".xml";

	emit(scaffoldChanged());
}

void GraphManager::setMesh( Model* model )
{
	this->entireMesh = (SurfaceMeshModel*) model;
	qDebug() << "Set active mesh as " << entireMesh->path;
}
