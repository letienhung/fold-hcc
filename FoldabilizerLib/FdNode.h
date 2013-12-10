#pragma once

#include "UtilityGlobal.h"
#include "Node.h"
#include "Box.h"
#include "ConvexHull.h"

class FdNode : public Structure::Node
{
public:
    FdNode(SurfaceMeshModel *m, Geom::Box &b);
	virtual void draw();

public:
	SurfaceMesh::SurfaceMeshModel *mesh;
	Geom::Box ctrlBox;
	QColor mColor;

	Geom::ConvexHull* CH;
};
