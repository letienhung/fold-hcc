#pragma once

#include "UtilityGlobal.h"

namespace Geom{

	class Frame2
	{
	public:
		Vector2 c;
		Vector2 r, s;

	public:
		// constructors
		Frame2();
		Frame2(Vector2& C);
		Frame2(Vector2& C, Vector2& X);
		Frame2(Vector2& C, Vector2& X, Vector2& Y);

		// coordinate
		Vector2 getCoords(Vector2& p);
		Vector2 getPosition(Vector2& coord);

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

}