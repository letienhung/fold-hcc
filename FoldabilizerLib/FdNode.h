#pragma once

#include "UtilityGlobal.h"
#include "Node.h"
#include "Box.h"
#include "XmlWriter.h"

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
	virtual void draw();

	// deformation
	virtual void updateBox();
	void encodeMesh();
	void deformMesh();

	// fit cuboid
	void refit(int method);

	// I/O
	void writeToXml(XmlWriter& xw);
	virtual void writeScaffoldToXml(XmlWriter& xw){}

public:
	Geom::Box origBox, mBox;
	MeshPtr mMesh;
	QVector<Vector3> meshCoords;

	QColor mColor;
	NODE_TYPE mType;
};
