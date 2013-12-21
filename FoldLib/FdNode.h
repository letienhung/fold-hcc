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
	FdNode(FdNode& other);
	~FdNode();

	virtual Node* clone();

	// visualization
	bool showCuboids;
	bool showScaffold;
	bool showMesh;
	void draw();
	void drawWithName(int name);

	// deformation
	void encodeMesh();
	void deformMesh();

	// fit cuboid
	virtual void refit(int method);

	// I/O
	void write(XmlWriter& xw);
	virtual void writeScaffold(XmlWriter& xw){Q_UNUSED(xw);}

	// geometry
	Geom::AABB computeAABB();
	Vector3 center();

	// relation with pushing direction
	virtual bool isPerpTo(Vector3 v, double dotThreshold);

public:
	Geom::Box origBox, mBox;
	MeshPtr mMesh;
	QVector<Vector3> meshCoords;

	QColor mColor;
	NODE_TYPE mType;

	// tags used for folding
	bool isCtrlPanel;
};

typedef QVector< QVector<FdNode*> > FdNodeArray2D;