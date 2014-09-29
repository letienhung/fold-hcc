#include "BundleNode.h"
#include "FdUtility.h"
#include "Numeric.h"

BundleNode::BundleNode( QString id, Geom::Box& b, QVector<ScaffoldNode*> nodes, Vector3 v )
	:PatchNode(id, b, MeshPtr(NULL),  v)
{
	foreach (ScaffoldNode* n, nodes)
		mNodes << (ScaffoldNode*)n->clone();

	// encode nodes
	Geom::Frame frame = mBox.getFrame();
	foreach (ScaffoldNode* n, mNodes)
		mNodeFrameRecords << frame.encodeFrame(n->mBox.getFrame());

	// inherits color from largest child
	double maxVol = -maxDouble();
	QColor maxColor;
	foreach (ScaffoldNode* n, mNodes)
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

BundleNode::BundleNode(BundleNode& other)
	:PatchNode(other)
{
	foreach(ScaffoldNode* n, other.mNodes)
		mNodes << (ScaffoldNode*)n->clone();

	mNodeFrameRecords = other.mNodeFrameRecords;
}


BundleNode::~BundleNode()
{
	foreach(ScaffoldNode* n, mNodes)
		delete n;
}


Structure::Node* BundleNode::clone()
{
	return new BundleNode(*this);
}

void BundleNode::drawMesh()
{
	deformMesh();
	foreach(ScaffoldNode* n, mNodes)
		n->drawMesh();
}

QString BundleNode::getMeshName()
{
	QString name;
	foreach (ScaffoldNode* n, mNodes)
		name += "+" + n->getMeshName();

	return name;
}

QVector<ScaffoldNode*> BundleNode::getSubNodes()
{
	QVector<ScaffoldNode*> pnodes;
	foreach (ScaffoldNode* n, mNodes)
		pnodes += n->getSubNodes();

	return pnodes;
}

ScaffoldNode* BundleNode::cloneChopped( Geom::Plane& chopper )
{
	// clone plain nodes
	QVector<ScaffoldNode*> plainNodes;
	foreach (ScaffoldNode* n, mNodes)
	{
		PLANE_RELATION relation = relationWithPlane(n, chopper, 0.1);
		if (relation == POS_PLANE)
			plainNodes << (ScaffoldNode*)n->clone();
		else if (relation == ISCT_PLANE)
			plainNodes << n->cloneChopped(chopper);
	}

	// return single plain fd node
	if (plainNodes.size() == 1)
	{
		return (ScaffoldNode*)plainNodes.front();
	}

	// return a bundle node
	if (plainNodes.size() > 1)
	{
		QString bid = getBundleName(plainNodes);
		Geom::Box box = getBundleBox(plainNodes);
		return new BundleNode(bid, box, plainNodes);

		// delete plain nodes
		foreach(ScaffoldNode* n, plainNodes)
			delete n;
	}

	return NULL;
}

ScaffoldNode* BundleNode::cloneChopped( Geom::Plane& chopper1, Geom::Plane& chopper2 )
{
	Geom::Plane plane1 = chopper1;
	Geom::Plane plane2 = chopper2;
	if (plane1.whichSide(plane2.Constant) < 0) plane1.flip();
	if (plane2.whichSide(plane1.Constant) < 0) plane2.flip();

	QVector<ScaffoldNode*> plainNodes;
	foreach (ScaffoldNode* n, mNodes)
	{
		PLANE_RELATION relation1 = relationWithPlane(n, plane1, 0.1);
		PLANE_RELATION relation2 = relationWithPlane(n, plane2, 0.1);
		if (relation1 == POS_PLANE && relation2 == POS_PLANE)
			plainNodes << (ScaffoldNode*)n->clone();
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
		return (ScaffoldNode*)plainNodes.front();
	}

	// return a bundle node
	if (plainNodes.size() > 1)
	{
		QString bid = getBundleName(plainNodes);
		Geom::Box box = getBundleBox(plainNodes);
		return new BundleNode(bid, box, plainNodes);

		// delete plain nodes
		foreach(ScaffoldNode* n, plainNodes)
			delete n;
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

void BundleNode::cloneMesh()
{
	deformMesh();
	foreach(ScaffoldNode* n, mNodes){
		n->cloneMesh();
	}
}

void BundleNode::exportMesh(QFile &file, int& v_offset)
{
	cloneMesh();
	foreach(ScaffoldNode* n, mNodes){
		n->exportMesh(file, v_offset);
	}
}

void BundleNode::setThickness( double thk )
{
	PatchNode::setThickness(thk);

	foreach (ScaffoldNode* n, mNodes)
		n->setThickness(thk);
}

void BundleNode::setShowCuboid( bool show )
{
	ScaffoldNode::setShowCuboid(show);
	foreach (ScaffoldNode* n, mNodes)
		n->setShowCuboid(show);
}

void BundleNode::setShowScaffold( bool show )
{
	ScaffoldNode::setShowScaffold(show);
	foreach (ScaffoldNode* n, mNodes)
		n->setShowScaffold(show);
}

void BundleNode::setShowMesh( bool show )
{
	ScaffoldNode::setShowMesh(show);
	foreach (ScaffoldNode* n, mNodes)
		n->setShowMesh(show);
}

void BundleNode::translate( Vector3 v )
{
	ScaffoldNode::translate(v);
	foreach (ScaffoldNode* n, mNodes)
		n->translate(v);
}

//void BundleNode::draw()
//{
//	foreach (ScaffoldNode* n, mNodes)
//	{
//		n->mColor = QColor::fromRgb(180, 180, 180);
//		n->mColor.setAlphaF(0.78);
//		n->draw();
//	}
//}
