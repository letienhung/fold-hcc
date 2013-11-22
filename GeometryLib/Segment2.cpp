#include "Segment2.h"

using namespace Geom;

Segment2::Segment2()
{
}

Segment2::Segment2(const Vector2& p0, const Vector2& p1)
{
	P0 = p0;
	P1 = p1;
	Center = (P0 + P1) / 2;
	Direction = P1 - P0;
	Extent = Direction.norm() / 2;
	Direction.normalize();
}
