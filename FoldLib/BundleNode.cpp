#include "BundleNode.h"
#include "FdUtility.h"

BundleNode::BundleNode( QString id, Geom::Box& b, QVector<FdNode*> nodes )
	:PatchNode(id, b, MeshPtr(NULL))
{
	foreach (FdNode* n, nodes)
		mNodes << (FdNode*)n->clone();

	// encode nodes
	Geom::Frame frame = mBox.getFrame();
	foreach (FdNode* n, mNodes)
		mNodeFrameRecords << frame.encodeFrame(n->mBox.getFrame());

	properties["isBundle"] = true;
}

BundleNode::BundleNode(BundleNode& other)
	:PatchNode(other)
{
	foreach(FdNode* n, other.mNodes)
		mNodes << (FdNode*)n->clone();

	mNodeFrameRecords = other.mNodeFrameRecords;
}


BundleNode::~BundleNode()
{
	foreach(FdNode* n, mNodes)
		delete n;
}


Structure::Node* BundleNode::clone()
{
	return new BundleNode(*this);
}

void BundleNode::drawMesh()
{
	deformMesh();
	foreach(FdNode* n, mNodes)
		n->drawMesh();
}

QString BundleNode::getMeshName()
{
	QString name;
	foreach (FdNode* n, mNodes)
		name += "+" + n->getMeshName();

	return name;
}

QVector<FdNode*> BundleNode::getPlainNodes()
{
	QVector<FdNode*> pnodes;
	foreach (FdNode* n, mNodes)
		pnodes += n->getPlainNodes();

	return pnodes;
}

FdNode* BundleNode::cloneChopped( Geom::Plane chopper )
{
	QVector<FdNode*> plainNodes;
	foreach (FdNode* n, mNodes)
	{
		PLANE_RELATION relation = relationWithPlane(n, chopper, 0.1);
		if (relation == POS_PLANE)
			plainNodes << n;
		else if (relation == ISCT_PLANE)
			plainNodes << n->cloneChopped(chopper);
	}

	// clone 
	if (plainNodes.size() == 1)
	{
		return (FdNode*)plainNodes.front()->clone();
	}

	if (plainNodes.size() > 1)
	{
		QString bid = getBundleName(plainNodes);
		Geom::Box box = getBundleBox(plainNodes);
		return new BundleNode(bid, box, plainNodes);
	}

	return NULL;
}

void BundleNode::deformMesh()
{
	Geom::Frame frame = mBox.getFrame();
	for (int i = 0; i < mNodes.size(); i++)
	{
		Geom::Frame nframe = frame.decodeFrame(mNodeFrameRecords[i]);
		mNodes[i]->mBox.setFrame(nframe);
		mNodes[i]->deformMesh();
	}
}
