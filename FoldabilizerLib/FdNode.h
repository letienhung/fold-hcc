#pragma once

#include "UtilityGlobal.h"
#include "Node.h"
#include "Box.h"
#include "XmlWriter.h"
#include "AABB.h"

class FdNode : public Structure::Node
{
public:
	enum NODE_TYPE{NONE, ROD, PATCH};

public:
    FdNode(MeshPtr m, Geom::Box &b);
	~FdNode();

	// visualization
	bool showCuboids;
	bool showScaffold;
	bool showMesh;
	void draw();
	void drawCuboid();
	void drawMesh();
	void drawWithName(int name);

	// deformation
	virtual void updateBox();
	void encodeMesh();
	void deformMesh();

	// fit cuboid
	virtual void refit(int method);

	// I/O
	void writeToXml(XmlWriter& xw);
	virtual void writeScaffoldToXml(XmlWriter& xw){Q_UNUSED(xw);}

	// aabb
	Geom::AABB computeAABB();

public:
	Geom::Box origBox, mBox;
	MeshPtr mMesh;
	QVector<Vector3> meshCoords;

	QColor mColor;
	NODE_TYPE mType;
};
