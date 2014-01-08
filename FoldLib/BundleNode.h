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

	QVector<FdNode*> getPlainNodes();

	FdNode* cloneChopped(Geom::Plane chopper);

public:
	QVector<FdNode*> mNodes;
	QVector<Geom::Frame::RecordInFrame> mNodeFrameRecords;
};

