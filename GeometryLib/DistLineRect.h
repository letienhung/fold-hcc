#include "Line.h"
#include "Rectangle.h"

namespace Geom{

class DistLineRect
{
public:
    DistLineRect(Line& line, Rectangle& rect);

	void compute();
	double get();
	double getSquared();

public:
	Line* mLine;
	Rectangle* mRectangle;

	Vector3 mClosestPoint0;
	Vector3 mClosestPoint1;

	// Information about the closest points.
	// closest0 = line.origin + param*line.direction
	double mLineParameter;
	// closest1 = rect.center + param0*rect.dir0 + param1*rect.dir1
	double mRectCoord[2];
};

}