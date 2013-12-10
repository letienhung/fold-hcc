#pragma once

#include "UtilityGlobal.h"
#include "Node.h"
#include "Box.h"
#include "xmlWriter.h"

class FdNode : public Structure::Node
{
public:
	enum NODE_TYPE{NONE, ROD, PATCH};

public:
    FdNode(SurfaceMeshModel *m, Geom::Box &b);

	// visualization
	bool showCuboids;
	bool showScaffold;
	virtual void draw();

	// deformation
	virtual void updateBox();
	void encodeMesh();
	void deformMesh();

	// I/O
	void writeToXml(XmlWriter& xw);
	virtual void writeScaffoldToXml(XmlWriter& xw){}
public:
	Geom::Box origBox, mBox;
	SurfaceMeshModel *mMesh;
	QVector<Vector3> meshCoords;

	QColor mColor;
	NODE_TYPE mType;
};
