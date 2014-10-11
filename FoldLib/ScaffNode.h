#pragma once
#include <QFile>

#include "UtilityGlobal.h"
#include "Node.h"
#include "Box.h"
#include "XmlWriter.h"
#include "AABB.h"
#include "FdUtility.h"

class ScaffNode : public Structure::Node
{
public: 
	enum NODE_TYPE{NONE, ROD, PATCH};

public:
	ScaffNode(QString id = "");
    ScaffNode(QString id, Geom::Box &b, MeshPtr m);
	ScaffNode(ScaffNode& other);
	virtual ~ScaffNode();

	Node* clone() = 0;

	// bundle node return all children, otherwise nothing
	virtual QVector<ScaffNode*> getSubNodes();

	// visualization
	bool isHidden;
	bool showCuboid;
	bool showScaffold;
	bool showMesh;
	void draw();
	virtual void drawMesh();
	virtual void drawScaffold() = 0;
	void drawWithName(int name);
	void setRandomColor();

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
	ScaffNode* cloneChopped(Geom::Box& chopBox);
	virtual ScaffNode* cloneChopped(Geom::Plane& chopper);
	virtual ScaffNode* cloneChopped(Geom::Plane& chopper1, Geom::Plane& chopper2);
	void deformToAttach(Geom::Plane& plane);
	virtual void setThickness(double thk);
	 
	// relation with direction
	virtual bool isPerpTo(Vector3 v, double dotThreshold);

	// visualization
	virtual void setShowCuboid(bool show);
	virtual void setShowScaffold(bool show);
	virtual void setShowMesh(bool show);

	// samples
	virtual QVector<Vector3> sampleBoundabyOfScaffold(int n) = 0;

public:
	Geom::Box origBox, mBox;
	MeshPtr mMesh;
	QVector<Vector3> meshCoords;

	QColor mColor;
	NODE_TYPE mType;
	int mAid; // axis index for creating scaffold
};

#define BUNDLE_TAG "isBundleNode"

#define SPLIT_ORIG "splitOriginNode"

#define EDGE_ROD_TAG "isEdgeRodNode"
#define EDGE_ROD_ORIG "edgeRodOriginNode"

#define	MASTER_TAG "isMasterPatch"

#define FIXED_NODE_TAG "hasFixed"

#define ACTIVE_NODE_TAG "isActiveNode"

#define MERGED_PART_TAG "isMergedNode"
#define MERGED_MASTERS "mergedMastersSet"
#define SUPER_MASTER_TAG "isSuperMaster"