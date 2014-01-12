#include "IntrRectRect.h"

#include "IntrSeg2Seg2.h"
#include "Numeric.h"


QVector<Vector3> Geom::IntrRectRect::test( Rectangle& rect0, Rectangle& rect1 )
{
	intPnts.clear();
	this->unilateralTest(rect0, rect1);
	this->unilateralTest(rect1, rect0);

	return intPnts;
}

void Geom::IntrRectRect::unilateralTest( Rectangle& rect0, Rectangle& rect1 )
{
	// corners and edges in 3D
	QVector<Vector3> conner0 = rect0.getConners();
	QVector<Segment> seg0 = rect0.getEdgeSegments();
	QVector<Segment> seg1 = rect1.getEdgeSegments();

	// edges in 2D
	QVector<Segment2> seg2d0, seg2d1;
	for (int i = 0; i < 4; i++)
	{
		seg2d0.push_back(rect1.getProjection2D(seg0[i]));
		seg2d1.push_back(rect1.getProjection2D(seg1[i]));
	}

	// test each corner against rect1
	QVector<bool> containsConner(4);
	for (int i = 0; i < 4; i++)
	{
		containsConner[i] = rect1.contains(conner0[i]);
		if (containsConner[i])
			this->addIntrPoint(conner0[i]);
	}

	// test each edge0 against all edges1
	for (int i = 0; i < 4; i++)
	{
		int next_i = (i + 1) % 4;
		
		// both ends are in, skip
		if ( containsConner[i] &&  containsConner[next_i]) continue;

		// one in one out or both out
		for (int j = 0; j < 4; j ++)
		{
			Vector2 it;
			int type = IntrSeg2Seg2::test(seg2d0[i], seg2d1[j], it);
			if (type == IntrSeg2Seg2::IT_POINT)
			{
				this->addIntrPoint(rect1.getPosition(it));
			}
		}
	}

}

void Geom::IntrRectRect::addIntrPoint( Vector3 p )
{
	foreach(Vector3 v, intPnts)
		if ((v-p).norm() < ZERO_TOLERANCE_LOW)
			return;

	intPnts.push_back(p);
}
