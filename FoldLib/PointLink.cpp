#include "PointLink.h"

PointLink::PointLink( FdNode* n1, FdNode* n2, Geom::Segment& distSeg )
	:FdLink(n1, n2, distSeg)
{
	mPos = mLink.Center;
}

PointLink::PointLink( PointLink& other )
	:FdLink(other)
{
	mPos = other.mPos;
}

Structure::Link* PointLink::clone()
{
	return new PointLink(*this);
}
