#include "IntrBoxBox.h"
#include "Numeric.h"


Geom::IntrBoxBox::IntrBoxBox()
{
}


bool Geom::IntrBoxBox::test( Box &b0, Box &b1, double s /*= 1.0*/ )
{
	Box box0 = b0.scaled(s);
	Box box1 = b1.scaled(s);

	// Cutoff for cosine of angles between box axes.  This is used to catch
	// the cases when at least one pair of axes are parallel.  If this
	// happens, there is no need to test for separation along the
	// Cross(A[i],B[j]) directions.
	const double cutoff = 1 - ZERO_TOLERANCE_LOW;
	bool existsParallelPair = false;
	int i;

	// Convenience variables.
	QVector<Vector3>& A = box0.Axis;
	QVector<Vector3>& B = box1.Axis;
	Vector3& EA = box0.Extent;
	Vector3& EB = box1.Extent;

	// Compute difference of box centers, D = C1-C0.
	Vector3 D = box1.Center - box0.Center;

	double C[3][3];     // matrix C = A^T B, c_{ij} = Dot(A_i,B_j)
	double AbsC[3][3];  // |c_{ij}|
	double AD[3];       // Dot(A_i,D)
	double r0, r1, r;   // interval radii and distance between centers
	double r01;         // = R0 + R1

	// axis C0+t*A0
	for (i = 0; i < 3; ++i)
	{
		C[0][i] = dot(A[0], B[i]);
		AbsC[0][i] = fabs(C[0][i]);
		if (AbsC[0][i] > cutoff)
		{
			existsParallelPair = true;
		}
	}
	AD[0] = dot(A[0], D);
	r = fabs(AD[0]);
	r1 = EB[0]*AbsC[0][0] + EB[1]*AbsC[0][1] + EB[2]*AbsC[0][2];
	r01 = EA[0] + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*A1
	for (i = 0; i < 3; ++i)
	{
		C[1][i] = dot(A[1], B[i]);
		AbsC[1][i] = fabs(C[1][i]);
		if (AbsC[1][i] > cutoff)
		{
			existsParallelPair = true;
		}
	}
	AD[1] = dot(A[1], D);
	r = fabs(AD[1]);
	r1 = EB[0]*AbsC[1][0] + EB[1]*AbsC[1][1] + EB[2]*AbsC[1][2];
	r01 = EA[1] + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*A2
	for (i = 0; i < 3; ++i)
	{
		C[2][i] = dot(A[2], B[i]);
		AbsC[2][i] = fabs(C[2][i]);
		if (AbsC[2][i] > cutoff)
		{
			existsParallelPair = true;
		}
	}
	AD[2] = dot(A[2], D);
	r = fabs(AD[2]);
	r1 = EB[0]*AbsC[2][0] + EB[1]*AbsC[2][1] + EB[2]*AbsC[2][2];
	r01 = EA[2] + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*B0
	r = fabs(dot(B[0], D));
	r0 = EA[0]*AbsC[0][0] + EA[1]*AbsC[1][0] + EA[2]*AbsC[2][0];
	r01 = r0 + EB[0];
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*B1
	r = fabs(dot(B[1], D));
	r0 = EA[0]*AbsC[0][1] + EA[1]*AbsC[1][1] + EA[2]*AbsC[2][1];
	r01 = r0 + EB[1];
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*B2
	r = fabs(dot(B[2], D));
	r0 = EA[0]*AbsC[0][2] + EA[1]*AbsC[1][2] + EA[2]*AbsC[2][2];
	r01 = r0 + EB[2];
	if (r > r01)
	{
		return false;
	}

	// At least one pair of box axes was parallel, so the separation is
	// effectively in 2D where checking the "edge" normals is sufficient for
	// the separation of the boxes.
	if (existsParallelPair)
	{
		return true;
	}

	// axis C0+t*A0xB0
	r = fabs(AD[2]*C[1][0] - AD[1]*C[2][0]);
	r0 = EA[1]*AbsC[2][0] + EA[2]*AbsC[1][0];
	r1 = EB[1]*AbsC[0][2] + EB[2]*AbsC[0][1];
	r01 = r0 + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*A0xB1
	r = fabs(AD[2]*C[1][1] - AD[1]*C[2][1]);
	r0 = EA[1]*AbsC[2][1] + EA[2]*AbsC[1][1];
	r1 = EB[0]*AbsC[0][2] + EB[2]*AbsC[0][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*A0xB2
	r = fabs(AD[2]*C[1][2] - AD[1]*C[2][2]);
	r0 = EA[1]*AbsC[2][2] + EA[2]*AbsC[1][2];
	r1 = EB[0]*AbsC[0][1] + EB[1]*AbsC[0][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*A1xB0
	r = fabs(AD[0]*C[2][0] - AD[2]*C[0][0]);
	r0 = EA[0]*AbsC[2][0] + EA[2]*AbsC[0][0];
	r1 = EB[1]*AbsC[1][2] + EB[2]*AbsC[1][1];
	r01 = r0 + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*A1xB1
	r = fabs(AD[0]*C[2][1] - AD[2]*C[0][1]);
	r0 = EA[0]*AbsC[2][1] + EA[2]*AbsC[0][1];
	r1 = EB[0]*AbsC[1][2] + EB[2]*AbsC[1][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*A1xB2
	r = fabs(AD[0]*C[2][2] - AD[2]*C[0][2]);
	r0 = EA[0]*AbsC[2][2] + EA[2]*AbsC[0][2];
	r1 = EB[0]*AbsC[1][1] + EB[1]*AbsC[1][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*A2xB0
	r = fabs(AD[1]*C[0][0] - AD[0]*C[1][0]);
	r0 = EA[0]*AbsC[1][0] + EA[1]*AbsC[0][0];
	r1 = EB[1]*AbsC[2][2] + EB[2]*AbsC[2][1];
	r01 = r0 + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*A2xB1
	r = fabs(AD[1]*C[0][1] - AD[0]*C[1][1]);
	r0 = EA[0]*AbsC[1][1] + EA[1]*AbsC[0][1];
	r1 = EB[0]*AbsC[2][2] + EB[2]*AbsC[2][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return false;
	}

	// axis C0+t*A2xB2
	r = fabs(AD[1]*C[0][2] - AD[0]*C[1][2]);
	r0 = EA[0]*AbsC[1][2] + EA[1]*AbsC[0][2];
	r1 = EB[0]*AbsC[2][1] + EB[1]*AbsC[2][0];
	r01 = r0 + r1;
	if (r > r01)
	{
		return false;
	}

	return true;
}

QVector<Vector3> Geom::IntrBoxBox::sampleIntr( Box &box0, Box &box1 )
{
	QVector<Vector3> pnts;

	if (IntrBoxBox::test(box0, box1))
	{
	}

	int N = 10;
	for (Vector3 p : box0.getGridSamples(N)){
		if (box1.contains(p)) pnts.push_back(p);
	}

	for (Vector3 p : box1.getGridSamples(N)){
		if (box0.contains(p)) pnts.push_back(p);
	}

	return pnts;
}
