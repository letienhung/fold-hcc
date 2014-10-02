#include "ScaffLink.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "CustomDrawObjects.h"

#include "FdUtility.h"
#include "DistSegSeg.h"
#include "DistSegRect.h"
#include "DistRectRect.h"
#include "Numeric.h"


ScaffLink::ScaffLink( ScaffNode* n1, ScaffNode* n2, Hinge* h)
	: Link(n1, n2)
{
	hinge = h;
}

ScaffLink::ScaffLink( ScaffLink& other )
	:Link(other)
{
	hinge = nullptr;
}

ScaffLink::~ScaffLink()
{
	if (hinge) delete hinge;
}

Structure::Link* ScaffLink::clone()
{
	return new ScaffLink(*this);
}

void ScaffLink::draw()
{
	if (hasTag(ACTIVE_LINK_TAG) && hinge)
	{
		hinge->draw();
	}
}

ScaffNode* ScaffLink::fix()
{
	if (hinge) 
		return hinge->fix();
	else
		return nullptr;
}
