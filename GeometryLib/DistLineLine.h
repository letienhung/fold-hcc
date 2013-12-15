#include "Line.h"

namespace Geom{

class DistLineLine
{
public:
    DistLineLine(Line& l0, Line& l1);

	void compute();
	double get();
	double getSquared();

public:
	Line *mLine0, *mLine1;

	Vector3 mClosestPoint0;
	Vector3 mClosestPoint1;
};

}