#include "FdLink.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "CustomDrawObjects.h"

FdLink::FdLink( FdNode* n1, FdNode* n2 )
	: Link(n1, n2)
{
	mLink.P0 = n1->mBox.Center;
	mLink.P1 = n2->mBox.Center; 
}

void FdLink::draw()
{
	mLink.draw(2.0, Qt::red);
}
