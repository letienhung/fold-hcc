#include "FdLink.h"

class LinearLink : public FdLink
{
public:
    LinearLink(FdNode* n1, FdNode* n2, Geom::Segment& distSeg);
	LinearLink(LinearLink& other);
	virtual Link* clone();
};

