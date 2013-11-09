#pragma once

#include "Box.h"

class IntersectBoxBox
{
public:
    IntersectBoxBox();
	~IntersectBoxBox(){}
	static bool test(Box &box0, Box &box1);
};

