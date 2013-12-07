#include "GraphManager.h"
#include "SegMeshLoader.h"
#include "PcaObb.h"
#include "RodNode.h"
#include "PatchNode.h"

GraphManager::GraphManager()
{
	this->scoffold = new FdGraph();
}

void GraphManager::createScoffold( SurfaceMesh::SurfaceMeshModel * entireMesh )
{
	SegMeshLoader sml(entireMesh);
	QVector<SurfaceMeshModel*> subMeshes = sml.getSegMeshes();

	this->scoffold->clear();
	Geom::PcaObb obb;
	foreach (SurfaceMeshModel* m, subMeshes)
	{
		// fit obb
		obb.build_from_mesh(m);
		Geom::Box box = obb.box();

		// create node depends on obb
		FdNode * node;
		int box_type = box.getType(5);
		if (box_type == Geom::Box::ROD)	
			 node = new RodNode(m->name);
		else node = new PatchNode(m->name);

		node->setMesh(m);
		node->setCtrlBox(box);

		// add to graph
		this->scoffold->addNode(node);
	}
}
