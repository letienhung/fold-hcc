#pragma once

#include "PatchNode.h"
#include "Frame.h"

class BundleNode : public PatchNode
{
public:
    BundleNode(QString id, Geom::Box& b, QVector<FdNode*> nodes);
	BundleNode(BundleNode& other);
	~BundleNode();

	virtual Node* clone();

	void drawMesh();
	QString getMeshName();
	void deformMesh();
	void cloneMesh();

	void exportMesh(QFile &file, int& v_offset);

	QVector<FdNode*> getPlainNodes();

	FdNode* cloneChopped(Geom::Plane& chopper);
	FdNode* cloneChopped(Geom::Plane& chopper1, Geom::Plane& chopper2);

public:
	QVector<FdNode*> mNodes;
	QVector<Geom::Frame::RecordInFrame> mNodeFrameRecords;
};

