#include "Line.h"
#include "Segment.h"


namespace Geom{

class DistLineSeg
{
public:
    DistLineSeg(Line& line, Segment& seg);

	void compute();
	double get();
	double getSquared();

public:
	Line* mLine;
	Segment* mSegment;

	Vector3 mClosestPoint0;
	Vector3 mClosestPoint1;

	double mLineParameter;
	double mSegmentParameter;
};

}