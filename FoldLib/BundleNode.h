#pragma once

#include "PatchNode.h"
#include "Frame.h"

class BundleNode : public PatchNode
{
public:
    BundleNode(QString id, Geom::Box& b, QVector<FdNode*> nodes, Vector3 v = Vector3(0, 0, 0));
	BundleNode(BundleNode& other);
	~BundleNode();

	virtual Node* clone();

	//void draw();
	void drawMesh();
	QString getMeshName();
	void deformMesh();
	void cloneMesh();

	void setThickness(double thk);

	void exportMesh(QFile &file, int& v_offset);

	QVector<FdNode*> getPlainNodes();

	FdNode* cloneChopped(Geom::Plane& chopper);
	FdNode* cloneChopped(Geom::Plane& chopper1, Geom::Plane& chopper2);

	// visual
	void setShowCuboid(bool show);
	void setShowScaffold(bool show);
	void setShowMesh(bool show);

	void translate(Vector3 v);

public:
	QVector<FdNode*> mNodes;
	QVector<Geom::Frame::RecordInFrame> mNodeFrameRecords;
};

