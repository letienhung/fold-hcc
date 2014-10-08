#include "IntrRect2Rect2.h"

bool Geom::IntrRect2Rect2::test( Rectangle2& rect1, Rectangle2& rect2 )
{
	// Convenience variables.
	QVector<Vector2> &A = rect1.Axis;
	QVector<Vector2> &B = rect2.Axis;
	Vector2 &EA = rect1.Extent;
	Vector2 &EB = rect2.Extent;

	// Compute difference of box centers, D = C1-C0.
	Vector2 D = rect1.Center - rect2.Center;

	double AbsAdB[2][2], AbsAdD, RSum;

	// axis C0+t*A0
	AbsAdB[0][0] = fabs(A[0].dot(B[0]));
	AbsAdB[0][1] = fabs(A[0].dot(B[1]));
	AbsAdD = fabs(A[0].dot(D));
	RSum = EA[0] + EB[0]*AbsAdB[0][0] + EB[1]*AbsAdB[0][1];
	if (AbsAdD > RSum)
	{
		return false;
	}

	// axis C0+t*A1
	AbsAdB[1][0] = fabs(A[1].dot(B[0]));
	AbsAdB[1][1] = fabs(A[1].dot(B[1]));
	AbsAdD =fabs(A[1].dot(D));
	RSum = EA[1] + EB[0]*AbsAdB[1][0] + EB[1]*AbsAdB[1][1];
	if (AbsAdD > RSum)
	{
		return false;
	}

	// axis C0+t*B0
	AbsAdD = fabs(B[0].dot(D));
	RSum = EB[0] + EA[0]*AbsAdB[0][0] + EA[1]*AbsAdB[1][0];
	if (AbsAdD > RSum)
	{
		return false;
	}

	// axis C0+t*B1
	AbsAdD = fabs(B[1].dot(D));
	RSum = EB[1] + EA[0]*AbsAdB[0][1] + EA[1]*AbsAdB[1][1];
	if (AbsAdD > RSum)
	{
		return false;
	}

	return true;

}

bool Geom::IntrRect2Rect2::test2(Rectangle2& rect1, Rectangle2& rect2)
{
	for (Vector2 p1 : rect1.getEdgeSamples(100)){
		if (rect2.contains(p1)) return true;
	}

	for (Vector2 p2 : rect2.getEdgeSamples(100)){
		if (rect1.contains(p2)) return true;
	}
	return false;
}
