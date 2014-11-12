#pragma once
#include <QFile>

#include "UtilityGlobal.h"
#include "Node.h"
#include "Box.h"
#include "XmlWriter.h"
#include "AABB.h"
#include "FdUtility.h"

class PatchNode;

class ScaffNode : public Structure::Node
{
public: 
	enum NODE_TYPE{NONE, ROD, PATCH};

public:
	ScaffNode(QString id = "");
    ScaffNode(QString id, Geom::Box &b, MeshPtr m);
	ScaffNode(ScaffNode& other);
	virtual ~ScaffNode();

	// visualization
	void drawWithName(int name);
	void setRandomColor();
	virtual void setColor(QColor c);
	virtual void draw() override;
	virtual void drawMesh();
	virtual void drawScaffold() = 0;

	// mesh
	void encodeMesh();
	virtual QString getMeshName();
	virtual void deformMesh();
	virtual void cloneMesh();

	// fit cuboid
	void refit(BOX_FIT_METHOD method);

	// I/O
	virtual void exportIntoXml(XmlWriter& xw);
	virtual void exportMeshIndividually(QString meshesFolder);
	virtual void exportIntoWholeMesh(QFile &wholeMeshFile, int& v_offset);

	// geometry
	Vector3 center();
	Geom::AABB computeAABB();
	virtual void createScaffold(bool useAid) = 0;

	// modifications
	// mesh is not touched, call deformMesh to update the location of mesh
	void deformToAttach(Geom::Plane& plane);
	void deformToAttach(PatchNode* pnode);
	void setBoxFrame(Geom::Frame frame);
	virtual void translate(Vector3 t);

	// chop
	ScaffNode* cloneChopped(Geom::Box& chopBox);
	virtual ScaffNode* cloneChopped(Geom::Plane& chopper);
	virtual ScaffNode* cloneChopped(Geom::Plane& chopper1, Geom::Plane& chopper2);
	 
	// relation with direction
	virtual bool isPerpTo(Vector3 v, double dotThreshold);

	// samples
	virtual QVector<Vector3> sampleBoundabyOfScaffold(int n) = 0;

public:
	Geom::Box origBox, mBox;
	MeshPtr mMesh;
	QVector<Vector3> meshCoords;

	QColor mColor;
	NODE_TYPE mType;
	int mAid; // axis index for creating scaffold

	// visualization
	bool isHidden;
};

#define BUNDLE_TAG "isBundleNode"

#define SPLIT_ORIG "splitOriginNode"

#define EDGE_VIRTUAL_TAG "isEdgeVirtualNode"
#define EDGE_ROD_ORIG "edgeRodOriginNode"

#define	MASTER_TAG "isMasterPatch"

#define FIXED_NODE_TAG "hasFixed"

#define ACTIVE_NODE_TAG "isActiveNode"

#define SUPER_PATCH_TAG "isSuperPatchNode"
#define MERGED_PART_TAG "isMergedPart"
