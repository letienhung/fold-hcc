#pragma once
#include <QFile>

#include "UtilityGlobal.h"
#include "Node.h"
#include "Box.h"
#include "XmlWriter.h"
#include "AABB.h"
#include "FdUtility.h"

class FdNode : public Structure::Node
{
public: 
	enum NODE_TYPE{NONE, ROD, PATCH};

public:
    FdNode(QString id, Geom::Box &b, MeshPtr m);
	FdNode(FdNode& other);
	~FdNode();

	virtual Node* clone() = 0;

	// plain nodes
	virtual QVector<FdNode*> getPlainNodes();

	// visualization
	bool isHidden;
	bool showCuboid;
	bool showScaffold;
	bool showMesh;
	void draw();
	virtual void drawMesh();
	virtual void drawScaffold() = 0;
	void drawWithName(int name);

	// mesh
	void encodeMesh();
	virtual void deformMesh();
	virtual QString getMeshName();
	virtual void cloneMesh();

	// fit cuboid
	void refit(BOX_FIT_METHOD method);

	// I/O
	void write(XmlWriter& xw);
	virtual void exportMesh(QFile &file, int& v_offset);

	// geometry
	virtual void createScaffold(bool useAid) = 0;
	Geom::AABB computeAABB();
	Vector3 center();

	// modification
	virtual void translate(Vector3 t);
	FdNode* cloneChopped(Geom::Box& chopBox);
	virtual FdNode* cloneChopped(Geom::Plane& chopper);
	virtual FdNode* cloneChopped(Geom::Plane& chopper1, Geom::Plane& chopper2);
	void deformToAttach(Geom::Plane& plane);
	virtual void setThickness(double thk);
	 
	// relation with direction
	virtual bool isPerpTo(Vector3 v, double dotThreshold);

	// visual
	virtual void setShowCuboid(bool show);
	virtual void setShowScaffold(bool show);
	virtual void setShowMesh(bool show);

public:
	Geom::Box origBox, mBox;
	MeshPtr mMesh;
	QVector<Vector3> meshCoords;

	QColor mColor;
	NODE_TYPE mType;
	int mAid; // axis index for creating scaffold
};