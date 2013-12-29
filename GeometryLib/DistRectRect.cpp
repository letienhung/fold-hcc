#include "DistRectRect.h"
#include "DistSegRect.h"
#include "Numeric.h"

Geom::DistRectRect::DistRectRect( Rectangle& rect0, Rectangle& rect1 )
{
	mRectangle0 = &rect0;
	mRectangle1 = &rect1;

	compute();
}

void Geom::DistRectRect::compute()
{
	// Compare edges of rectangle0 to the interior of rectangle1.
	double sqrDist = maxDouble(), sqrDistTmp;
	Segment edge;
	int i0, i1;
	for (i1 = 0; i1 < 2; ++i1)
	{
		for (i0 = -1; i0 <= 1; i0 += 2)
		{
			edge.Center = mRectangle0->Center +
				(i0*mRectangle0->Extent[1-i1]) *
				mRectangle0->Axis[1-i1];
			edge.Direction = mRectangle0->Axis[i1];
			edge.Extent = mRectangle0->Extent[i1];
			edge.computeEndPoints();

			DistSegRect querySR(edge, *mRectangle1);
			sqrDistTmp = querySR.getSquared();
			if (sqrDistTmp < sqrDist)
			{
				mClosestPoint0 = querySR.mClosestPoint0;
				mClosestPoint1 = querySR.mClosestPoint1;
				sqrDist = sqrDistTmp;
			}
		}
	}

	// Compare edges of rectangle1 to the interior of rectangle0.
	for (i1 = 0; i1 < 2; ++i1)
	{
		for (i0 = -1; i0 <= 1; i0 += 2)
		{
			edge.Center = mRectangle1->Center +
				(i0*mRectangle1->Extent[1-i1]) *
				mRectangle1->Axis[1-i1];
			edge.Direction = mRectangle1->Axis[i1];
			edge.Extent = mRectangle1->Extent[i1];
			edge.computeEndPoints();

			DistSegRect querySR(edge, *mRectangle0);
			sqrDistTmp = querySR.getSquared();
			if (sqrDistTmp < sqrDist)
			{
				mClosestPoint0 = querySR.mClosestPoint0;
				mClosestPoint1 = querySR.mClosestPoint1;
				sqrDist = sqrDistTmp;
			}
		}
	}
}


double Geom::DistRectRect::get()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.norm();
}

double Geom::DistRectRect::getSquared()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.squaredNorm();
}