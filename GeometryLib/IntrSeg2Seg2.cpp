#include "IntrSeg2Seg2.h"
#include "Numeric.h"

using namespace Geom;

int IntrSeg2Seg2::test( const Segment2& segment0, const Segment2& segment1, Vector2& it )
{
	// The intersection of two lines is a solution to P0+s0*D0 = P1+s1*D1.
	// Rewrite this as s0*D0 - s1*D1 = P1 - P0 = Q.  
	// If D0.Dot(Perp(D1)) = 0, the lines are parallel.  
	// Additionally, if Q.Dot(Perp(D1)) = 0, the lines are the same.  
	// If D0.Dot(Perp(D1)) is not zero, then
	//   s0 = Q.Dot(Perp(D1))/D0.Dot(Perp(D1))
	// produces the point of intersection.  Also,
	//   s1 = Q.Dot(Perp(D0))/D0.Dot(Perp(D1))

	Vector2 originDiff = segment1.Center - segment0.Center;

	double D0DotPerpD1 = dotPerp(segment0.Direction, segment1.Direction);

	// Lines are not parallel and thus intersect in a single point
	if (fabs(D0DotPerpD1) > ZERO_TOLERANCE_LOW)
	{
		// compute the intersection
		double invD0DotPerpD1 = 1.0 / D0DotPerpD1;
		double diffDotPerpD0 = dotPerp(originDiff, segment0.Direction);
		double diffDotPerpD1 = dotPerp(originDiff, segment1.Direction);
		double s0 = diffDotPerpD1 * invD0DotPerpD1;
		double s1 = diffDotPerpD0 * invD0DotPerpD1;

		// verify whether the line intersection is on both segments
		if (   fabs(s0) <= segment0.Extent + ZERO_TOLERANCE_LOW
			&& fabs(s1) <= segment1.Extent + ZERO_TOLERANCE_LOW)
		{
			it = segment0.Center + s0 * segment0.Direction;
			return IT_POINT;
		}
		else
			return IT_EMPTY;
	}

	// lines are parallel
	else
	{
		double diffNDotPerpD1 = dotPerp(originDiff, segment1.Direction);
		if (fabs(diffNDotPerpD1) <= ZERO_TOLERANCE_LOW)
			// Lines are collinear
			return IT_COLLINEAR;
		else
			// Lines are distinct
			return IT_EMPTY;
	}

}