#include "UtilityGlobal.h"

class MeshMerger
{
public:
    MeshMerger();

	void addMesh(SurfaceMeshModel* subMesh);
	SurfaceMeshModel* getMesh();

private:
	SurfaceMeshModel* mergedMesh;
};