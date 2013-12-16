#include "FdLink.h"

class PointLink : public FdLink
{
public:
    PointLink(FdNode* n1, FdNode* n2);

	PointLink(PointLink& other);
	virtual Link* clone();

public:
	Point mPos;
};

