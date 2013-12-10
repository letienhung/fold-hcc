#include "GraphManager.h"
#include "SegMeshLoader.h"
#include "RodNode.h"
#include "PatchNode.h"

#include "PcaObb.h"
#include "AABB.h"
#include "MinOBB.h"

GraphManager::GraphManager()
{
	this->scaffold = new FdGraph();
}

void GraphManager::createScaffold( SurfaceMesh::SurfaceMeshModel * entireMesh )
{
	SegMeshLoader sml(entireMesh);
	QVector<SurfaceMeshModel*> subMeshes = sml.getSegMeshes();

	// reset scaffold
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
}
