#include "FdLink.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "CustomDrawObjects.h"

#include "FdUtility.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "DistRectRect.h"
#include "Numeric.h"


FdLink::FdLink( FdNode* n1, FdNode* n2)
	: Link(n1, n2)
{
}

FdLink::FdLink( FdLink& other )
	:Link(other)
{
}

FdLink::~FdLink()
{
}

Structure::Link* FdLink::clone()
{
	return new FdLink(*this);
}

void FdLink::draw()
{
}
