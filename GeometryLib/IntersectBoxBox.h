#pragma once

#include "Box.h"

namespace Goem{

class IntersectBoxBox
{
public:
    IntersectBoxBox();
	~IntersectBoxBox(){}
	static bool test(Box &box0, Box &box1);
};

}