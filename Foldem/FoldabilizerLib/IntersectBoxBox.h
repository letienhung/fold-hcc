#pragma once

#include "Box.h"

class IntersectBoxBox
{
public:
    IntersectBoxBox(Box &box0, Box &box1);

	//bool test();

private:
	Box *mBox0, mBox1; 
};

