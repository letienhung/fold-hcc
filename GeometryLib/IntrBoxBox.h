#pragma once

#include "Box.h"

namespace Geom{

class IntrBoxBox
{
public:
    IntrBoxBox();
    ~IntrBoxBox(){}
	static bool test(Box &box0, Box &box1);
};

}
