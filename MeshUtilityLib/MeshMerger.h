#include "UtilityGlobal.h"

class MeshMerger
{
public:
    MeshMerger();

	void addMesh(SurfaceMeshModel* subMesh);
	SurfaceMeshModel* getMesh();

private:
	QString name;
	SurfaceMeshModel* mergedMesh;
};