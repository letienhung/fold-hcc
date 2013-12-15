#include "Rectangle.h"

namespace Geom{

class DistRectRect
{
public:
	DistRectRect(Rectangle& rect0, Rectangle& rect1);

	void compute();

	double get();
	double getSquared();

public:
	Rectangle* mRectangle0;
	Rectangle* mRectangle1;

	Vector3 mClosestPoint0;
	Vector3 mClosestPoint1;
};

}