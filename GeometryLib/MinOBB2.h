// Geometric Tools, LLC

#pragma once

#include "ConvexHull2.h"

typedef double Real;

namespace Geom{

class  MinOBB2
{
public:
	class Box2
	{
	public:
		Vector2 Center;
		QVector<Vector2> Axis;
		Vector2 Extent;
	public:
		Box2(){Axis.resize(2);}
		double area(){return Extent[0] * Extent[1];}
	};

public:
	MinOBB2 (QVector<Vector2> &pnts);
	Box2 getBox2();


private:
    // Flags for the rotating calipers algorithm.
    enum { F_NONE, F_LEFT, F_RIGHT, F_BOTTOM, F_TOP };

    void UpdateBox (const Vector2& LPoint,
        const Vector2& RPoint, const Vector2& BPoint,
        const Vector2& TPoint, const Vector2& U,
        const Vector2& V, Real& minAreaDiv4);


private:
	Box2 mMinBox;
};


}