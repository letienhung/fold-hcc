#include "Segment.h"

namespace Geom{

class DistSegSeg
{
public:
    DistSegSeg(Segment& seg0, Segment& seg1);

	void compute();
	double get();
	double getSquared();

public:
	Segment mSegment0, mSegment1;

	Vector3 mClosestPoint0;
	Vector3 mClosestPoint1;
};

}