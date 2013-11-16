#include "ContactBoxBox.h"

ContactBoxBox::ContactBoxBox()
{
}

ContactBoxBox::ContactBoxBox( Box &b0, Box &b1 )
	:box0(b0), box1(b1)
{
}

BoxBoxRelation ContactBoxBox::getBoxBoxRelation()
{



	return LINE_LINE;
}