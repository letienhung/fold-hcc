#include "FdLink.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "CustomDrawObjects.h"

#include "FdUtility.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "DistRectRect.h"
#include "Numeric.h"


FdLink::FdLink( FdNode* n1, FdNode* n2, Hinge* h)
	: Link(n1, n2)
{
	hinge = h;
}

FdLink::FdLink( FdLink& other )
	:Link(other)
{
	// to do: clone hinge
}

FdLink::~FdLink()
{
	delete hinge;
}

Structure::Link* FdLink::clone()
{
	return new FdLink(*this);
}

void FdLink::draw()
{
	if (properties["active"].toBool() && hinge)
	{
		hinge->draw();
	}
}

bool FdLink::fix()
{
	if (hinge) 
		return hinge->fix();

	// to avoid infinity loop
	node1->properties["fixed"] = true;
	node2->properties["fixed"] = true;
	return true;
}
