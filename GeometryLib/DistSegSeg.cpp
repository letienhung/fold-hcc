#include "DistSegSeg.h"
#include "Numeric.h"

Geom::DistSegSeg::DistSegSeg(Segment& seg0, Segment& seg1)
{
	mSegment0 = seg0;
	mSegment1 = seg1;

	compute();
}

void Geom::DistSegSeg::compute()
{
	Vector3 diff = mSegment0.Center - mSegment1.Center;
	double a01 = -mSegment0.Direction.dot(mSegment1.Direction);
	double b0 = diff.dot(mSegment0.Direction);
	double b1 = -diff.dot(mSegment1.Direction);
	double c = diff.squaredNorm();
	double det = fabs((double)1 - a01*a01);
	double s0, s1, extDet0, extDet1, tmpS0, tmpS1;

	if (det >= ZERO_TOLERANCE_LOW)
	{
		// Segments are not parallel.
		s0 = a01*b1 - b0;
		s1 = a01*b0 - b1;
		extDet0 = mSegment0.Extent*det;
		extDet1 = mSegment1.Extent*det;

		if (s0 >= -extDet0)
		{
			if (s0 <= extDet0)
			{
				if (s1 >= -extDet1)
				{
					if (s1 <= extDet1)  // region 0 (interior)
					{
						// Minimum at interior points of segments.
						double invDet = ((double)1)/det;
						s0 *= invDet;
						s1 *= invDet;
					}
					else  // region 3 (side)
					{
						s1 = mSegment1.Extent;
						tmpS0 = -(a01*s1 + b0);
						if (tmpS0 < -mSegment0.Extent)
						{
							s0 = -mSegment0.Extent;
						}
						else if (tmpS0 <= mSegment0.Extent)
						{
							s0 = tmpS0;
						}
						else
						{
							s0 = mSegment0.Extent;
						}
					}
				}
				else  // region 7 (side)
				{
					s1 = -mSegment1.Extent;
					tmpS0 = -(a01*s1 + b0);
					if (tmpS0 < -mSegment0.Extent)
					{
						s0 = -mSegment0.Extent;
					}
					else if (tmpS0 <= mSegment0.Extent)
					{
						s0 = tmpS0;
					}
					else
					{
						s0 = mSegment0.Extent;
					}
				}
			}
			else
			{
				if (s1 >= -extDet1)
				{
					if (s1 <= extDet1)  // region 1 (side)
					{
						s0 = mSegment0.Extent;
						tmpS1 = -(a01*s0 + b1);
						if (tmpS1 < -mSegment1.Extent)
						{
							s1 = -mSegment1.Extent;
						}
						else if (tmpS1 <= mSegment1.Extent)
						{
							s1 = tmpS1;
						}
						else
						{
							s1 = mSegment1.Extent;
						}
					}
					else  // region 2 (corner)
					{
						s1 = mSegment1.Extent;
						tmpS0 = -(a01*s1 + b0);
						if (tmpS0 < -mSegment0.Extent)
						{
							s0 = -mSegment0.Extent;
						}
						else if (tmpS0 <= mSegment0.Extent)
						{
							s0 = tmpS0;
						}
						else
						{
							s0 = mSegment0.Extent;
							tmpS1 = -(a01*s0 + b1);
							if (tmpS1 < -mSegment1.Extent)
							{
								s1 = -mSegment1.Extent;
							}
							else if (tmpS1 <= mSegment1.Extent)
							{
								s1 = tmpS1;
							}
							else
							{
								s1 = mSegment1.Extent;
							}
						}
					}
				}
				else  // region 8 (corner)
				{
					s1 = -mSegment1.Extent;
					tmpS0 = -(a01*s1 + b0);
					if (tmpS0 < -mSegment0.Extent)
					{
						s0 = -mSegment0.Extent;
					}
					else if (tmpS0 <= mSegment0.Extent)
					{
						s0 = tmpS0;
					}
					else
					{
						s0 = mSegment0.Extent;
						tmpS1 = -(a01*s0 + b1);
						if (tmpS1 > mSegment1.Extent)
						{
							s1 = mSegment1.Extent;
						}
						else if (tmpS1 >= -mSegment1.Extent)
						{
							s1 = tmpS1;
						}
						else
						{
							s1 = -mSegment1.Extent;
						}
					}
				}
			}
		}
		else 
		{
			if (s1 >= -extDet1)
			{
				if (s1 <= extDet1)  // region 5 (side)
				{
					s0 = -mSegment0.Extent;
					tmpS1 = -(a01*s0 + b1);
					if (tmpS1 < -mSegment1.Extent)
					{
						s1 = -mSegment1.Extent;
					}
					else if (tmpS1 <= mSegment1.Extent)
					{
						s1 = tmpS1;
					}
					else
					{
						s1 = mSegment1.Extent;
					}
				}
				else  // region 4 (corner)
				{
					s1 = mSegment1.Extent;
					tmpS0 = -(a01*s1 + b0);
					if (tmpS0 > mSegment0.Extent)
					{
						s0 = mSegment0.Extent;
					}
					else if (tmpS0 >= -mSegment0.Extent)
					{
						s0 = tmpS0;
					}
					else
					{
						s0 = -mSegment0.Extent;
						tmpS1 = -(a01*s0 + b1);
						if (tmpS1 < -mSegment1.Extent)
						{
							s1 = -mSegment1.Extent;
						}
						else if (tmpS1 <= mSegment1.Extent)
						{
							s1 = tmpS1;
						}
						else
						{
							s1 = mSegment1.Extent;
						}
					}
				}
			}
			else   // region 6 (corner)
			{
				s1 = -mSegment1.Extent;
				tmpS0 = -(a01*s1 + b0);
				if (tmpS0 > mSegment0.Extent)
				{
					s0 = mSegment0.Extent;
				}
				else if (tmpS0 >= -mSegment0.Extent)
				{
					s0 = tmpS0;
				}
				else
				{
					s0 = -mSegment0.Extent;
					tmpS1 = -(a01*s0 + b1);
					if (tmpS1 < -mSegment1.Extent)
					{
						s1 = -mSegment1.Extent;
					}
					else if (tmpS1 <= mSegment1.Extent)
					{
						s1 = tmpS1;
					}
					else
					{
						s1 = mSegment1.Extent;
					}
				}
			}
		}
	}
	else
	{
		// The segments are parallel.  The average b0 term is designed to
		// ensure symmetry of the function.  That is, dist(seg0,seg1) and
		// dist(seg1,seg0) should produce the same number.
		double e0pe1 = mSegment0.Extent + mSegment1.Extent;
		double sign = (a01 > (double)0 ? (double)-1 : (double)1);
		double b0Avr = ((double)0.5)*(b0 - sign*b1);
		double lambda = -b0Avr;
		if (lambda < -e0pe1)
		{
			lambda = -e0pe1;
		}
		else if (lambda > e0pe1)
		{
			lambda = e0pe1;
		}

		s1 = -sign*lambda*mSegment1.Extent/e0pe1;
		s0 = lambda + sign*s1;
	}

	mClosestPoint0 = mSegment0.Center + s0*mSegment0.Direction;
	mClosestPoint1 = mSegment1.Center + s1*mSegment1.Direction;
}


double Geom::DistSegSeg::get()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.norm();
}

double Geom::DistSegSeg::getSquared()
{
	Vector3 diff = mClosestPoint0 - mClosestPoint1;
	return diff.squaredNorm();
}