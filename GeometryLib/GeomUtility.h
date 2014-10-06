#pragma once

#include "Rectangle2.h"

namespace Geom{

	// 2D bounding box
	Rectangle2 computeAABB(QVector<Vector2> &pnts);
	Rectangle2 computeBoundingBox(QVector<Vector2>& pnts, Vector2& X);


}