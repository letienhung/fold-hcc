#pragma once

#include "UtilityGlobal.h"
#include "Node.h"
#include "Box.h"

class ScoffoldNode : public Structure::Node
{
public:
    ScoffoldNode(QString nid);

	void setMesh(SurfaceMesh::SurfaceMeshModel *m);
	void setCtrlBox(Geom::Box &b);

public:
	SurfaceMesh::SurfaceMeshModel *mesh;
	Geom::Box ctrlBox;
};
