// Geometric Tools, LLC
#include "MinOBB.h"
#include "Numeric.h"
#include "AABB.h"
#include "ProbabilityDistributions.h"


Geom::MinOBB::MinOBB( QVector<Vector3> &points, bool addNoise)
{
	if (addNoise) computeWithNoise(points);
	else compute(points);
}

void Geom::MinOBB::computeWithNoise( QVector<Vector3> &points )
{
	// Add noise to reduce precision problems in CH
	AABB aabb(points);
	double enlarge_scale = 1.0/100;
	double noise_scale = enlarge_scale / 2 * aabb.radius();

	for (int i = 0; i < (int)points.size(); i++)
	{
		points[i] -= aabb.center();
		points[i] *= (1 + enlarge_scale);
		double nx = uniformDistrReal() - 0.5;
		double ny = uniformDistrReal() - 0.5;
		double nz = uniformDistrReal() - 0.5;
		points[i] += Vector3(nx, ny,nz) * noise_scale;
		points[i] += aabb.center();
	}

	// Compute minOBB
	compute(points);
}

void Geom::MinOBB::compute( QVector<Vector3> &points )
{
 	isReady = false;

   // Get the convex hull of the points.
    ConvexHull kHull(points);
 
    int hullQuantity = kHull.getNumSimplices();
    QVector<int> hullIndices = kHull.getIndices();
    Real volume, minVolume = maxDouble();

    // Create the unique set of hull vertices to minimize the time spent
    // projecting vertices onto planes of the hull faces.
    std::set<int> uniqueIndices(hullIndices.begin(), hullIndices.end());

	int i, j;
	Vector3 origin, diff, U, V, W;
    QVector<Vector2> points2(uniqueIndices.size(), Vector2());
	Box2 box2;
    // Use the rotating calipers method on the projection of the hull onto
    // the plane of each face.  Also project the hull onto the normal line
    // of each face.  The minimum area box in the plane and the height on
    // the line produce a containing box.  If its volume is smaller than the
    // current volume, this box is the new candidate for the minimum volume
    // box.  The unique edges are accumulated into a set for use by a later
    // step in the algorithm.
    QVector<int>::const_iterator currentHullIndex = hullIndices.begin();
    Real height, minHeight, maxHeight;
    std::set<EdgeKey> edges;

    for (i = 0; i < hullQuantity; ++i)
    {
        // Get the triangle.
        int v0 = *currentHullIndex++;
        int v1 = *currentHullIndex++;
        int v2 = *currentHullIndex++;

        // Save the edges for later use.
        edges.insert(EdgeKey(v0, v1));
        edges.insert(EdgeKey(v1, v2));
        edges.insert(EdgeKey(v2, v0));

        // Get 3D coordinate system relative to plane of triangle.
        origin = (points[v0] + points[v1] + points[v2])/(Real)3.0;
		U = points[v2] - points[v0];
        V = points[v1] - points[v0];
		U.normalize();	        
		V.normalize();
		W = cross(U, V);  // inner-pointing normal
		if (W.norm() < ZERO_TOLERANCE_LOW)
		{
			continue; // The triangle is needle-like, so skip it.
		}
		W.normalize();
		V = cross(W, U);
        

        // Project points onto plane of triangle, onto normal line of plane.
        minHeight = (Real)0;
        maxHeight = (Real)0;
        j = 0;
		std::set<int>::const_iterator iter = uniqueIndices.begin();
		while (iter != uniqueIndices.end())
        {
            int index = *iter++;
            diff = points[index] - origin;
            points2[j].x() = dot(U, diff);
            points2[j].y() = dot(V, diff);
            height = dot(W, diff);
            if (height > maxHeight)
            {
                maxHeight = height;
            }

            j++;
        }

        // Compute minimum area box in 2D.
        MinOBB2 mobb(points2);
		box2 = mobb.getBox2();

        // Update current minimum-volume box (if necessary).
		volume = maxHeight*box2.Extent[0]*box2.Extent[1];
        if (volume < minVolume)
        {
            minVolume = volume;

            // Lift the values into 3D.
            mMinBox.Extent[0] = box2.Extent[0];
            mMinBox.Extent[1] = box2.Extent[1];
            mMinBox.Extent[2] = ((Real)0.5)*maxHeight;
            mMinBox.Axis[0] = box2.Axis[0].x()*U + box2.Axis[0].y()*V;
            mMinBox.Axis[1] = box2.Axis[1].x()*U + box2.Axis[1].y()*V;
            mMinBox.Axis[2] = W;
            mMinBox.Center = origin + box2.Center.x()*U + box2.Center.y()*V
                + mMinBox.Extent[2]*W;
        }
    }

    // The minimum-volume box can also be supported by three mutually
    // orthogonal edges of the convex hull.  For each triple of orthogonal
    // edges, compute the minimum-volume box for that coordinate frame by
    // projecting the points onto the axes of the frame.
    std::set<EdgeKey>::const_iterator e2iter;
    for (e2iter = edges.begin(); e2iter != edges.end(); e2iter++)
    {
        W = points[e2iter->V[1]] - points[e2iter->V[0]];
        W.normalize();

        std::set<EdgeKey>::const_iterator e1iter = e2iter;
        for (++e1iter; e1iter != edges.end(); e1iter++)
        {
            V = points[e1iter->V[1]] - points[e1iter->V[0]];
            V.normalize();
            if (fabs(dot(V, W)) > ZERO_TOLERANCE_LOW)
            {
                continue;
            }

            std::set<EdgeKey>::const_iterator e0iter = e1iter;
            for (++e0iter; e0iter != edges.end(); e0iter++)
            {
                U = points[e0iter->V[1]] - points[e0iter->V[0]];
                U.normalize();
                if (fabs(dot(U,V)) > ZERO_TOLERANCE_LOW)
                {
                    continue;
                }
                if (fabs(dot(U,W)) > ZERO_TOLERANCE_LOW)
                {
                    continue;
                }
    
                // The three edges are mutually orthogonal.  Project the
                // hull points onto the lines containing the edges.  Use
                // hull point zero as the origin.
                Real umin = (Real)0, umax = (Real)0;
                Real vmin = (Real)0, vmax = (Real)0;
                Real wmin = (Real)0, wmax = (Real)0;
                origin = points[hullIndices[0]];

                std::set<int>::const_iterator iter = uniqueIndices.begin();
                while (iter != uniqueIndices.end())
                {
                    int index = *iter++;
                    diff = points[index] - origin;

                    Real fU = dot(U, diff);
                    if (fU < umin)
                    {
                        umin = fU;
                    }
                    else if (fU > umax)
                    {
                        umax = fU;
                    }

                    Real fV = dot(V, diff);
                    if (fV < vmin)
                    {
                        vmin = fV;
                    }
                    else if (fV > vmax)
                    {
                        vmax = fV;
                    }

                    Real fW = dot(W, diff);
                    if (fW < wmin)
                    {
                        wmin = fW;
                    }
                    else if (fW > wmax)
                    {
                        wmax = fW;
                    }
                }

                Real uExtent = ((Real)0.5)*(umax - umin);
                Real vExtent = ((Real)0.5)*(vmax - vmin);
                Real wExtent = ((Real)0.5)*(wmax - wmin);

                // Update current minimum-volume box (if necessary).
                volume = uExtent*vExtent*wExtent;
                if (volume < minVolume)
                {
                    minVolume = volume;

                    mMinBox.Extent[0] = uExtent;
                    mMinBox.Extent[1] = vExtent;
                    mMinBox.Extent[2] = wExtent;
                    mMinBox.Axis[0] = U;
                    mMinBox.Axis[1] = V;
                    mMinBox.Axis[2] = W;
                    mMinBox.Center = origin +
                        ((Real)0.5)*(umin+umax)*U +
                        ((Real)0.5)*(vmin+vmax)*V +
                        ((Real)0.5)*(wmin+wmax)*W;
                }
            }
        }
    }
	isReady = true;
}

void Geom::MinOBB::GenerateComplementBasis (Vector3& u, Vector3& v, const Vector3& w)
{
	Real invLength;

	if (fabs(w[0]) >= fabs(w[1]))
	{
		// W.x or W.z is the largest magnitude component, swap them
		invLength = (Real)1/sqrt(w[0]*w[0] + w[2]*w[2]);
		u[0] = -w[2]*invLength;
		u[1] = (Real)0;
		u[2] = +w[0]*invLength;
		v[0] = w[1]*u[2];
		v[1] = w[2]*u[0] - w[0]*u[2];
		v[2] = -w[1]*u[0];
	}
	else
	{
		// W.y or W.z is the largest magnitude component, swap them
		invLength = (Real)1/sqrt(w[1]*w[1] + w[2]*w[2]);
		u[0] = (Real)0;
		u[1] = +w[2]*invLength;
		u[2] = -w[1]*invLength;
		v[0] = w[1]*u[2] - w[2]*u[1];
		v[1] = -w[0]*u[2];
		v[2] = w[0]*u[1];
	}
}

void Geom::MinOBB::getCorners( QVector<Vector3> &pnts )
{
	pnts.resize(8);

	if ( dot(cross(mMinBox.Axis[0], mMinBox.Axis[1]), mMinBox.Axis[2]) < 0 ) 
	{
		mMinBox.Axis[2]  = -mMinBox.Axis[2];
	}

	QVector<Vector3> Axis;
	for (int i=0;i<3;i++)
	{
		Axis.push_back( 2 * mMinBox.Extent[i] * mMinBox.Axis[i]);
	}

	pnts[0] = mMinBox.Center - 0.5*Axis[0] - 0.5*Axis[1] + 0.5*Axis[2];
	pnts[1] = pnts[0] + Axis[0];
	pnts[2] = pnts[1] - Axis[2];
	pnts[3] = pnts[2] - Axis[0];

	pnts[4] = pnts[0] + Axis[1];
	pnts[5] = pnts[1] + Axis[1];
	pnts[6] = pnts[2] + Axis[1];
	pnts[7] = pnts[3] + Axis[1];
}

Geom::MinOBB::EdgeKey::EdgeKey (int v0, int v1)
{
	if (v0 < v1)
	{
		// v0 is minimum
		V[0] = v0;
		V[1] = v1;
	}
	else
	{
		// v1 is minimum
		V[0] = v1;
		V[1] = v0;
	}
}
//----------------------------------------------------------------------------
bool Geom::MinOBB::EdgeKey::operator< (const EdgeKey& key) const
{
	if (V[1] < key.V[1])
	{
		return true;
	}

	if (V[1] > key.V[1])
	{
		return false;
	}

	return V[0] < key.V[0];
}
//----------------------------------------------------------------------------
Geom::MinOBB::EdgeKey::operator size_t () const
{
	return V[0] | (V[1] << 16);
}