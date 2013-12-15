#include "Segment.h"
#include "Rectangle.h"

namespace Geom{

class DistSegRect
{
public:
    DistSegRect(Segment& seg, Rectangle& rect);

	void compute();

public:
	Segment* mSegment;
	Rectangle* mRectangle;

	Vector3 mClosestPoint0;
	Vector3 mClosestPoint1;

	// Information about the closest points.

	// closest0 = seg.origin + param*seg.direction
	double mSegmentParameter;

	// closest1 = rect.center + param0*rect.dir0 + param1*rect.dir1
	double mRectCoord[2];
};

}