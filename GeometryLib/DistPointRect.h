#include "Rectangle.h"

namespace Geom{

class DistPointRect
{
public:
	DistPointRect(Vector3& point, Rectangle& rect);

	void compute();

	double get();
	double getSquared();

public:
	Vector3* mPoint;
	Rectangle* mRectangle;

	Vector3 mClosestPoint0;
	Vector3 mClosestPoint1;

	// Information about the closest points.
	// closest1 = rect.center + param0*rect.dir0 + param1*rect.dir1
	double mRectCoord[2];
};

}