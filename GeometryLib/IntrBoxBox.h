#pragma once

#include "Box.h"

namespace Goem{

class IntrBoxBox
{
public:
    IntrBoxBox();
    ~IntrBoxBox(){}
	static bool test(Box &box0, Box &box1);
};

}
