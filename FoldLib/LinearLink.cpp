#include "LinearLink.h"

LinearLink::LinearLink( FdNode* n1, FdNode* n2, Geom::Segment& distSeg )
	: FdLink(n1, n2, distSeg)
{

}

LinearLink::LinearLink( LinearLink& other )
	:FdLink(other)
{

}

Structure::Link* LinearLink::clone()
{
	return new LinearLink(*this);
}
