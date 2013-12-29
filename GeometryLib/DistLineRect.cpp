#include "DistLineRect.h"
#include "Numeric.h"
#include "DistLineSeg.h"

Geom::DistLineRect::DistLineRect( Line& line, Rectangle& rect )
{
	mLine = &line;
	mRectangle = &rect;

	compute();
}

void Geom::DistLineRect::compute()
{
	// Test if line intersects rectangle.  If so, the squared distance is
	// zero.
	Vector3 N = mRectangle->Axis[0].cross(mRectangle->Axis[1]);
	double NdD = N.dot(mLine->Direction);
	if (fabs(NdD) > ZERO_TOLERANCE_LOW)
	{
		// The line and rectangle are not parallel, so the line intersects
		// the plane of the rectangle.
		Vector3 diff = mLine->Origin - mRectangle->Center;
		Vector3 U, V;
		generateComplementBasis(U, V, mLine->Direction);
		double UdD0 = U.dot(mRectangle->Axis[0]);
		double UdD1 = U.dot(mRectangle->Axis[1]);
		double UdPmC = U.dot(diff);
		double VdD0 = V.dot(mRectangle->Axis[0]);
		double VdD1 = V.dot(mRectangle->Axis[1]);
		double VdPmC = V.dot(diff);
		double invDet = ((double)1)/(UdD0*VdD1 - UdD1*VdD0);

		// Rectangle coordinates for the point of intersection.
		double s0 = (VdD1*UdPmC - UdD1*VdPmC)*invDet;
		double s1 = (UdD0*VdPmC - VdD0*UdPmC)*invDet;

		if (fabs(s0) <= mRectangle->Extent[0]
		&&  fabs(s1) <= mRectangle->Extent[1])
		{
			// Line parameter for the point of intersection.
			double DdD0 = mLine->Direction.dot(mRectangle->Axis[0]);
			double DdD1 = mLine->Direction.dot(mRectangle->Axis[1]);
			double DdDiff = mLine->Direction.dot(diff);
			mLineParameter = s0*DdD0 + s1*DdD1 - DdDiff;

			// Rectangle coordinates for the point of intersection.
			mRectCoord[0] = s0;
			mRectCoord[1] = s1;

			// The intersection point is inside or on the rectangle.
			mClosestPoint0 = mLine->Origin +
				mLineParameter*mLine->Direction;

			mClosestPoint1 = mRectangle->Center +
				s0*mRectangle->Axis[0] + s1*mRectangle->Axis[1];

			return;
		}
	}

	// Either (1) the line is not parallel to the rectangle and the point of
	// intersection of the line and the plane of the rectangle is outside the
	// rectangle or (2) the line and rectangle are parallel.  Regardless, the
	// closest point on the rectangle is on an edge of the rectangle.  Compare
	// the line to all four edges of the rectangle.
	double sqrDist = maxDouble();
	Vector3 scaledDir[2] =
	{
		mRectangle->Extent[0]*mRectangle->Axis[0],
		mRectangle->Extent[1]*mRectangle->Axis[1]
	};
	for (int i1 = 0; i1 < 2; ++i1)
	{
		for (int i0 = 0; i0 < 2; ++i0)
		{
			Segment segment;
			segment.Center = mRectangle->Center +
				((double)(2*i0-1))*scaledDir[i1];
			segment.Direction = mRectangle->Axis[1-i1];
			segment.Extent = mRectangle->Extent[1-i1];
			segment.computeEndPoints();

			DistLineSeg queryLS(*mLine, segment);
			double sqrDistTmp = queryLS.getSquared();
			if (sqrDistTmp < sqrDist)
			{
				mClosestPoint0 = queryLS.mClosestPoint0;
				mClosestPoint1 = queryLS.mClosestPoint1;
				sqrDist = sqrDistTmp;

				mLineParameter = queryLS.mLineParameter;
				double ratio = queryLS.mSegmentParameter/segment.Extent;
				mRectCoord[0] = mRectangle->Extent[0]*((1-i1)*(2*i0-1) + i1*ratio);
				mRectCoord[1] = mRectangle->Extent[1]*((1-i0)*(2*i1-1) + i0*ratio);
			}
		}
	}
}


double Geom::DistLineRect::get()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.norm();
}

double Geom::DistLineRect::getSquared()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.squaredNorm();
}