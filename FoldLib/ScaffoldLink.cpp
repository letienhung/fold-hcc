#include "ScaffoldLink.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "CustomDrawObjects.h"

#include "FdUtility.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "DistRectRect.h"
#include "Numeric.h"


ScaffoldLink::ScaffoldLink( ScaffoldNode* n1, ScaffoldNode* n2, Hinge* h)
	: Link(n1, n2)
{
	hinge = h;
}

ScaffoldLink::ScaffoldLink( ScaffoldLink& other )
	:Link(other)
{
	hinge = NULL;
}

ScaffoldLink::~ScaffoldLink()
{
	if (hinge) delete hinge;
}

Structure::Link* ScaffoldLink::clone()
{
	return new ScaffoldLink(*this);
}

void ScaffoldLink::draw()
{
	if (hasTag(ACTIVE_LINK_TAG) && hinge)
	{
		hinge->draw();
	}
}

ScaffoldNode* ScaffoldLink::fix()
{
	if (hinge) 
		return hinge->fix();
	else
		return NULL;
}
