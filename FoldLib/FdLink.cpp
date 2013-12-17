#include "FdLink.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "CustomDrawObjects.h"

FdLink::FdLink( FdNode* n1, FdNode* n2, Geom::Segment& distSeg )
	: Link(n1, n2)
{
	mLink = distSeg;
}

FdLink::FdLink( FdLink& other )
	:Link(other)
{
	mLink = other.mLink;
}

Structure::Link* FdLink::clone()
{
	return new FdLink(*this);
}

void FdLink::draw()
{
	mLink.draw(2.0, Qt::red);
}


