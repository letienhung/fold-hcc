#pragma once

#include "Box.h"

enum BoxBoxRelation{
	LINE_LINE, LINE_FACE, WHOLE_WHOLE, WHOLE_PART, PART_PART
};

class ContactBoxBox
{
public:
    ContactBoxBox();
	ContactBoxBox(Box &box1, Box &box2);

	BoxBoxRelation getBoxBoxRelation();

private:
	Box box0, box1;
};


