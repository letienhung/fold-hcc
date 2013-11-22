#pragma once

#include "Segment2.h"

namespace Geom {

class IntrSeg2Seg2
{
public:
	enum {IT_EMPTY, IT_POINT, IT_COLLINEAR};

	static int test(const Segment2& segment0, const Segment2& segment1, Vector2& it);
};

}
