#pragma once

#include "Rectangle.h"

namespace Geom{

class IntrRectRect
{
public:
	QVector<Vector3> intPnts;

	QVector<Vector3> test(Rectangle& rect0, Rectangle& rect1);

	void addIntrPoint(Vector3 p);
	void unilateralTest(Rectangle& rect0, Rectangle& rect1);
};

}