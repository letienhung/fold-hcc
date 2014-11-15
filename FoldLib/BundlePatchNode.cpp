#include "BundlePatchNode.h"
#include "FdUtility.h"
#include "Numeric.h"

BundlePatchNode::BundlePatchNode( QString id, Geom::Box& b, QVector<ScaffNode*> nodes, Vector3 v )
	:PatchNode(id, b, MeshPtr(nullptr),  v)
{
	for (ScaffNode* n : nodes)
		subNodes << (ScaffNode*)n->clone();

	// encode nodes
	Geom::Frame frame = mBox.getFrame();
	for (ScaffNode* n : subNodes)
		subNodeFrameRecords << frame.encodeFrame(n->mBox.getFrame());

	// inherits color from largest child
	double maxVol = -maxDouble();
	QColor maxColor;
	for (ScaffNode* n : subNodes)
	{
		if (n->mBox.volume() > maxVol)
		{
			maxVol = n->mBox.volume();
			maxColor = n->mColor;
		}
	}
	mColor = maxColor;

	addTag(BUNDLE_TAG);
}

BundlePatchNode::BundlePatchNode(BundlePatchNode& other)
	:PatchNode(other)
{
	for (ScaffNode* n : other.subNodes)
		subNodes << (ScaffNode*)n->clone();

	subNodeFrameRecords = other.subNodeFrameRecords;
}


BundlePatchNode::~BundlePatchNode()
{
	for (ScaffNode* n : subNodes)
		delete n;
}


Structure::Node* BundlePatchNode::clone()
{
	return new BundlePatchNode(*this);
}

void BundlePatchNode::drawMesh()
{
	deformMesh();
	for (ScaffNode* n : subNodes)
		n->drawMesh();
}

QString BundlePatchNode::getMeshName()
{
	QString name;
	for (ScaffNode* n : subNodes)
		name += "+" + n->getMeshName();

	return name;
}

ScaffNode* BundlePatchNode::cloneChoppedBetween(Vector3 p0, Vector3 p1)
{
	QVector<ScaffNode*> cns;
	for (ScaffNode* n : subNodes)
	{
		ScaffNode* cn = n->cloneChoppedBetween(p0, p1);
		if (cn) cns << cn;
	}

	if (cns.isEmpty)
	{
		return nullptr;
	}
	else if (cns.size() == 1)
	{
		return cns.front();
	}
	else
	{
		QString bid = getBundleName(cns);
		Geom::Box box = getBundleBox(cns);
		ScaffNode* bn =  new BundlePatchNode(bid, box, cns);
		for (ScaffNode* cn : cns) delete cn;
		return bn;
	}
}

void BundlePatchNode::deformMesh()
{
	restoreSubNodes();
	for (ScaffNode* node : subNodes)
		node->deformMesh();
}

void BundlePatchNode::cloneMesh()
{
	deformMesh();
	for (ScaffNode* n : subNodes){
		n->cloneMesh();
	}
}

void BundlePatchNode::exportIntoWholeMesh(QFile &file, int& v_offset)
{
	cloneMesh();
	for (ScaffNode* n : subNodes){
		n->exportIntoWholeMesh(file, v_offset);
	}
}

void BundlePatchNode::translate( Vector3 v )
{
	ScaffNode::translate(v);
	for (ScaffNode* n : subNodes)
		n->translate(v);
}

void BundlePatchNode::exportIntoXml(XmlWriter& xw)
{
	QStringList subNodeNames;

	// save each sub-node
	for (ScaffNode* node : subNodes)
	{
		node->exportIntoXml(xw);
		subNodeNames << node->mID;
	}

	// the bundle
	xw.writeTaggedString("bundle", subNodeNames.join(' '));
}

void BundlePatchNode::exportMeshIndividually(QString meshesFolder)
{
	for (ScaffNode* node : subNodes)
		node->exportMeshIndividually(meshesFolder);
}

void BundlePatchNode::restoreSubNodes()
{
	Geom::Frame bundleFrame = mBox.getFrame();
	for (int i = 0; i < subNodes.size(); i++)
	{
		ScaffNode* sn = subNodes[i];
		Geom::Frame::RecordInFrame snfr = subNodeFrameRecords[i];
		sn->setBoxFrame(bundleFrame.decodeFrame(snfr));
	}
}

void BundlePatchNode::setColor(QColor c)
{
	// the patch
	ScaffNode::setColor(c);

	// all sub nodes
	for (ScaffNode* n : subNodes)
	{
		n->setColor(c);
	}
}
