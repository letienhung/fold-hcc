#include "PatchNode.h"
#include "Segment.h"
#include "Box.h"
#include "CustomDrawObjects.h"

PatchNode::PatchNode(MeshPtr m, Geom::Box &b)
	: FdNode(m, b)
{
	mType = FdNode::PATCH;
	createPatch();
}


PatchNode::~PatchNode()
{

}


void PatchNode::createPatch()
{
	int aid = mBox.minAxisId();
	QVector<Geom::Segment> edges = mBox.getEdgeSegmentsAlongAxis(aid);

	QVector<Vector3> conners;
	for (int i = 0; i < 4; i++)	conners.push_back(edges[i].Center);
	mPatch = Geom::Rectangle(conners);
}



void PatchNode::draw()
{
	if (showScaffold)
	{
		QColor c = Qt::red;
		mPatch.draw(c.lighter());
	}

	FdNode::draw();
}

void PatchNode::refit(int method)
{
	FdNode::refit(method);

	// update patch
	createPatch();
}
