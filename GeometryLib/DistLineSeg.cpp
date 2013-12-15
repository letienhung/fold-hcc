#include "DistLineSeg.h"
#include "Numeric.h"

Geom::DistLineSeg::DistLineSeg( Line& line, Segment& seg )
{
	mLine = &line;
	mSegment = &seg;

	compute();
}

void Geom::DistLineSeg::compute()
{
	Vector3 diff = mLine->Origin - mSegment->Center;
	double a01 = -mLine->Direction.dot(mSegment->Direction);
	double b0 = diff.dot(mLine->Direction);
	double c = diff.squaredNorm();
	double det = fabs((double)1 - a01*a01);
	double b1, s0, s1, extDet;

	if (det >= ZERO_TOLERANCE_LOW)
	{
		// The line and segment are not parallel.
		b1 = -diff.dot(mSegment->Direction);
		s1 = a01*b0 - b1;
		extDet = mSegment->Extent*det;

		if (s1 >= -extDet)
		{
			if (s1 <= extDet)
			{
				// Two interior points are closest, one on the line and one
				// on the segment.
				double invDet = ((double)1)/det;
				s0 = (a01*b1 - b0)*invDet;
				s1 *= invDet;
			}
			else
			{
				// The endpoint e1 of the segment and an interior point of
				// the line are closest.
				s1 = mSegment->Extent;
				s0 = -(a01*s1 + b0);
			}
		}
		else
		{
			// The end point e0 of the segment and an interior point of the
			// line are closest.
			s1 = -mSegment->Extent;
			s0 = -(a01*s1 + b0);
		}
	}
	else
	{
		// The line and segment are parallel.  Choose the closest pair so that
		// one point is at segment center.
		s1 = (double)0;
		s0 = -b0;
	}

	mClosestPoint0 = mLine->Origin + s0*mLine->Direction;
	mClosestPoint1 = mSegment->Center + s1*mSegment->Direction;

	mLineParameter = s0;
	mSegmentParameter = s1;
}


double Geom::DistLineSeg::get()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.norm();
}

double Geom::DistLineSeg::getSquared()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.squaredNorm();
}