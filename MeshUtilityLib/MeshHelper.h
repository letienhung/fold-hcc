#pragma once
#include "UtilityGlobal.h"
#include "Box.h"

class MeshHelper
{
public:
	static QVector<Vector3> getMeshVertices(SurfaceMeshModel* mesh);
    static void saveOBJ(SurfaceMeshModel* mesh, QString filename);
	static QVector<Vector3> encodeMeshInBox(SurfaceMeshModel* mesh, Geom::Box& box);
	static bool decodeMeshInBox(SurfaceMeshModel* mesh, Geom::Box& box, QVector<Vector3>& coords);
	static void deformMeshByBoxes(SurfaceMeshModel* mesh, Geom::Box& fromBox, Geom::Box& toBox);
	static SurfaceMeshModel* loadMesh(QString filename);
};

