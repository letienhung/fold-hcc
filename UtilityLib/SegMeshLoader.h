#pragma once

#include "UtilityGlobal.h"

class SegMeshLoader
{
public:
    SegMeshLoader(SurfaceMeshModel * mesh);

	// Entire mesh
	SurfaceMesh::SurfaceMeshModel* entireMesh;
	Vector3VertexProperty entirePoints;

	// Groups
	QMap< QString, QVector<int> > groupFaces;

	void loadGroupsFromObj();
	MeshPtr extractSegMesh( QString gid );
	QVector<MeshPtr> getSegMeshes();
};

