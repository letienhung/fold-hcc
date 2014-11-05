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

ScaffNode* BundlePatchNode::cloneChopped( Geom::Plane& chopper )
{
	// clone plain nodes
	QVector<ScaffNode*> plainNodes;
	for (ScaffNode* n : subNodes)
	{
		PLANE_RELATION relation = relationWithPlane(n, chopper, 0.1);
		if (relation == POS_PLANE)
			plainNodes << (ScaffNode*)n->clone();
		else if (relation == ISCT_PLANE)
			plainNodes << n->cloneChopped(chopper);
	}

	// return single plain fd node
	if (plainNodes.size() == 1)
	{
		return (ScaffNode*)plainNodes.front();
	}

	// return a bundle node
	if (plainNodes.size() > 1)
	{
		QString bid = getBundleName(plainNodes);
		Geom::Box box = getBundleBox(plainNodes);
		return new BundlePatchNode(bid, box, plainNodes);

		// delete plain nodes
		for (ScaffNode* n : plainNodes)
			delete n;
	}

	return nullptr;
}

ScaffNode* BundlePatchNode::cloneChopped( Geom::Plane& chopper1, Geom::Plane& chopper2 )
{
	Geom::Plane plane1 = chopper1;
	Geom::Plane plane2 = chopper2;
	if (plane1.whichSide(plane2.Constant) < 0) plane1.flip();
	if (plane2.whichSide(plane1.Constant) < 0) plane2.flip();

	QVector<ScaffNode*> plainNodes;
	for (ScaffNode* n : subNodes)
	{
		PLANE_RELATION relation1 = relationWithPlane(n, plane1, 0.1);
		PLANE_RELATION relation2 = relationWithPlane(n, plane2, 0.1);
		if (relation1 == POS_PLANE && relation2 == POS_PLANE)
			plainNodes << (ScaffNode*)n->clone();
		else if (relation1 == ISCT_PLANE && relation2 == ISCT_PLANE)
			plainNodes << n->cloneChopped(plane1, plane2);
		else if (relation1 == ISCT_PLANE)
			plainNodes << n->cloneChopped(plane1);
		else if (relation2 == ISCT_PLANE)
			plainNodes << n->cloneChopped(plane2);
	}

	// return single plain fd node
	if (plainNodes.size() == 1)
	{
		return (ScaffNode*)plainNodes.front();
	}

	// return a bundle node
	if (plainNodes.size() > 1)
	{
		QString bid = getBundleName(plainNodes);
		Geom::Box box = getBundleBox(plainNodes);
		return new BundlePatchNode(bid, box, plainNodes);

		// delete plain nodes
		for (ScaffNode* n : plainNodes)
			delete n;
	}

	return nullptr;
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

void BundlePatchNode::setShowCuboid( bool show )
{
	ScaffNode::setShowCuboid(show);
	for (ScaffNode* n : subNodes)
		n->setShowCuboid(show);
}

void BundlePatchNode::setShowScaffold( bool show )
{
	ScaffNode::setShowScaffold(show);
	for (ScaffNode* n : subNodes)
		n->setShowScaffold(show);
}

void BundlePatchNode::setShowMesh( bool show )
{
	ScaffNode::setShowMesh(show);
	for (ScaffNode* n : subNodes)
		n->setShowMesh(show);
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
