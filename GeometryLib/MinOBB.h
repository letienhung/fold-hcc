// From Geometric Tools, LLC

#pragma once

#include "ConvexHull.h"
#include "MinOBB2.h"
#include "Box.h"

namespace Geom{

class  MinOBB
{
public:
	MinOBB(QVector<Vector3> &points, bool addNoise);

	void compute( QVector<Vector3> &points );
	void computeWithNoise(QVector<Vector3> &points);
	void GenerateComplementBasis (Vector3& u, Vector3& v, const Vector3& w);
	void getCorners( QVector<Vector3> &pnts );

	Vec3d ClosestPtPointOBB(const Vec3d& p);

private:
	class EdgeKey
	{
	public:
		EdgeKey (int v0 = -1, int v1 = -1);

		bool operator< (const EdgeKey& key) const;
		operator size_t () const;

		int V[2];
	};
    

public:
	Box mMinBox;
	bool isReady;
};

}