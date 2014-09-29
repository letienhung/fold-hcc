#pragma once

#include "ScaffoldNode.h"
#include "Link.h"
#include "Hinge.h"
#include "RodNode.h"
#include "PatchNode.h"

class ScaffoldLink : public Structure::Link
{
public:
    ScaffoldLink(ScaffoldNode* n1, ScaffoldNode* n2, Hinge* h = NULL);
	ScaffoldLink(ScaffoldLink& other);
	~ScaffoldLink();

	Link* clone();

	ScaffoldNode* fix();
	void draw();

public:
	Hinge* hinge;
};

#define ACTIVE_LINK_TAG "isActiveLink"