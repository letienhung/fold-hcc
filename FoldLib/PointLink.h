#include "FdLink.h"

class PointLink : public FdLink
{
public:
    PointLink(FdNode* n1, FdNode* n2);

public:
	Point mPos;
};

