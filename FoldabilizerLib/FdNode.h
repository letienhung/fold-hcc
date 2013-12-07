#pragma once

#include "UtilityGlobal.h"
#include "Node.h"
#include "Box.h"

class FdNode : public Structure::Node
{
public:
    FdNode(QString nid);

	void setMesh(SurfaceMesh::SurfaceMeshModel *m);
	void setCtrlBox(Geom::Box &b);

public:
	SurfaceMesh::SurfaceMeshModel *mesh;
	Geom::Box ctrlBox;
};
