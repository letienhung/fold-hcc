#pragma once

#include "Rectangle2.h"

namespace Geom{

	class IntrRect2Rect2
	{
	public:
		static bool test(Rectangle2& rect1, Rectangle2& rect2);
	};


}