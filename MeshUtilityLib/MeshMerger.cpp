#include "MeshMerger.h"
#include "MeshHelper.h"

MeshMerger::MeshMerger()
{
	mergedMesh = new SurfaceMeshModel();
	meshName = "";
}

SurfaceMeshModel* MeshMerger::getMesh()
{
	mergedMesh->update_face_normals();
	mergedMesh->update_vertex_normals();
	mergedMesh->updateBoundingBox();

	mergedMesh->name = meshName;

	return mergedMesh;
}

void MeshMerger::addMesh( SurfaceMeshModel* subMesh )
{
	int offset = mergedMesh->n_vertices();
	
	for (Vector3 p : MeshHelper::getMeshVertices(subMesh))
	{
		mergedMesh->add_vertex(p);
	}

	Surface_mesh::Face_iterator fit, fend = subMesh->faces_end();
	for (fit = subMesh->faces_begin(); fit != fend; fit++)
	{
		std::vector<Vertex> pnts; 
		Surface_mesh::Vertex_around_face_circulator vit = subMesh->vertices(fit), vend = vit;
		do{ 
			int sub_vid = Vertex(vit).idx();
			pnts.push_back(Vertex(sub_vid + offset)); 
		} while(++vit != vend);

		mergedMesh->add_face(pnts);
	}

	meshName += "+" + subMesh->name;
}
