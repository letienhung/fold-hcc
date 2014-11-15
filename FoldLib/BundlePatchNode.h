#pragma once

#include "PatchNode.h"
#include "Frame.h"

class BundlePatchNode final : public PatchNode
{
public:
    BundlePatchNode(QString id, Geom::Box& b, QVector<ScaffNode*> nodes, Vector3 v = Vector3(0, 0, 0));
	BundlePatchNode(BundlePatchNode& other);
	~BundlePatchNode();

	virtual Node* clone() override;

	// meshes
	virtual void drawMesh() override;
	virtual void deformMesh() override;
	virtual void cloneMesh() override;
	virtual	QString getMeshName() override;

	// I/O
	virtual void exportIntoXml(XmlWriter& xw) override;
	virtual void exportMeshIndividually(QString meshesFolder) override;
	virtual void exportIntoWholeMesh(QFile &file, int& v_offset) override;

	// chop
	virtual ScaffNode* cloneChoppedBetween(Vector3 p0, Vector3 p1) override;

	// visual
	virtual void setColor(QColor c) override;

	// transformation
	virtual void translate(Vector3 v) override;

	// sub-nodes
	void restoreSubNodes();

public:
	QVector<ScaffNode*> subNodes;
	QVector<Geom::Frame::RecordInFrame> subNodeFrameRecords;
};

