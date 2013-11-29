#pragma once

#include "Box.h"

namespace Geom{

class IntrBoxBox
{
public:
    IntrBoxBox();
    ~IntrBoxBox(){}
	
	static bool test(Box &b0, Box &b1, double s = 1.0);
	static QVector<Vector3> sampleIntr(Box &box0, Box &box1);
};

}
