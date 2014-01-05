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

	virtual Node* clone() = 0;
	void setStringId(QString id);

	// visualization
	bool showCuboids;
	bool showScaffold;
	bool showMesh;
	void draw();
	void drawWithName(int name);

	// mesh
	void encodeMesh();
	void deformMesh();

	// fit cuboid
	void refit(int method);

	// I/O
	void write(XmlWriter& xw);

	// geometry
	virtual void createScaffold() = 0;
	Geom::AABB computeAABB();
	Vector3 center();
	FdNode* cloneChopped(Geom::Plane chopper);
	 
	// relation with direction
	virtual bool isPerpTo(Vector3 v, double dotThreshold);

public:
	Geom::Box origBox, mBox;
	MeshPtr mMesh;
	QVector<Vector3> meshCoords;

	QColor mColor;
	NODE_TYPE mType;
};

typedef QVector< QVector<FdNode*> > FdNodeArray2D;