#include "DistSegRect.h"
#include "DistLineRect.h"
#include "DistPointRect.h"

Geom::DistSegRect::DistSegRect(Segment& seg, Rectangle& rect)
{
	mSegment = &seg;
	mRectangle = &rect;

	compute();
}

void Geom::DistSegRect::compute()
{
	Line line(mSegment->Center, mSegment->Direction);
	DistLineRect queryLR(line, *mRectangle);
	mSegmentParameter = queryLR.mLineParameter;

	if (mSegmentParameter >= -mSegment->Extent)
	{
		if (mSegmentParameter <= mSegment->Extent)
		{
			mClosestPoint0 = queryLR.mClosestPoint0;
			mClosestPoint1 = queryLR.mClosestPoint1;
			mRectCoord[0] = queryLR.mRectCoord[0];
			mRectCoord[1] = queryLR.mRectCoord[1];
		}
		else
		{
			mClosestPoint0 = mSegment->P1;
			DistPointRect queryPR(mClosestPoint0, *mRectangle);
			mClosestPoint1 = queryPR.mClosestPoint1;
			mSegmentParameter = mSegment->Extent;
			mRectCoord[0] = queryPR.mRectCoord[0];
			mRectCoord[1] = queryPR.mRectCoord[1];
		}
	}
	else
	{
		mClosestPoint0 = mSegment->P0;
		DistPointRect queryPR(mClosestPoint0, *mRectangle);
		mClosestPoint1 = queryPR.mClosestPoint1;
		mSegmentParameter = -mSegment->Extent;
		mRectCoord[0] = queryPR.mRectCoord[0];
		mRectCoord[1] = queryPR.mRectCoord[1];
	}
}


double Geom::DistSegRect::get()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.norm();
}

double Geom::DistSegRect::getSquared()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.squaredNorm();
}