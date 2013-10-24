#pragma once

#include "Box.h"

class IntersectBoxBox
{
public:
	IntersectBoxBox(){}
    IntersectBoxBox(Box &box0, Box &box1);
	~IntersectBoxBox(){}
	//bool test();

private:
	Box mBox0, mBox1; 
};

