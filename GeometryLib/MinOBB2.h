#pragma once

#include "ConvexHull2.h"
#include "Rectangle2.h"

typedef double Real;

namespace Geom{

class  MinOBB2
{
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