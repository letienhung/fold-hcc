#include "PatchNode.h"
#include "Segment.h"
#include "Box.h"
#include "CustomDrawObjects.h"

PatchNode::PatchNode(SurfaceMeshModel *m, Geom::Box &b)
	: FdNode(m, b)
{
	mType = FdNode::PATCH;

	int aid = mBox.minAxisId();
	QVector<Geom::Segment> edges = mBox.getEdgeSegmentsAlongAxis(aid);

	QVector<Vector3> conners;
	for (int i = 0; i < 4; i++)
	{
		conners.push_back(edges[i].Center);
	}

	mPatch = Geom::Rectangle(conners);
}

void PatchNode::draw()
{
	if (showScaffold)
	{
		PolygonSoup ps;
		ps.addPoly(mPatch.Conners);
		ps.draw();
	}

	FdNode::draw();
}
