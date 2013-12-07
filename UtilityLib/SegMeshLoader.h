#pragma once

#include "UtilityGlobal.h"

class SegMeshLoader
{
public:
    SegMeshLoader();

	// Groups
	SurfaceMesh::SurfaceMeshModel * entireMesh;
	QMap< QString, QVector<int> > groupFaces;
	QMap< int, QString > faceGroup;
	void loadGroupsFromOBJ();
};

