#pragma once

#include "UtilityGlobal.h"
#include "Node.h"
#include "Box.h"

class FdNode : public Structure::Node
{
public:
	enum NODE_TYPE{NONE, ROD, PATCH};

public:
    FdNode(SurfaceMeshModel *m, Geom::Box &b);
	virtual void draw();

public:
	SurfaceMeshModel *mesh;
	Geom::Box mBox;
	QColor mColor;
	NODE_TYPE mType;
};
