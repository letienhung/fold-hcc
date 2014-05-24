#pragma once

#include "FdNode.h"
#include "Link.h"
#include "Hinge.h"
#include "RodNode.h"
#include "PatchNode.h"

class FdLink : public Structure::Link
{
public:
    FdLink(FdNode* n1, FdNode* n2, Hinge* h = NULL);
	FdLink(FdLink& other);
	~FdLink();

	Link* clone();

	FdNode* fix();
	void draw();

public:
	Hinge* hinge;
};


