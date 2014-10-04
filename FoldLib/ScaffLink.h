#pragma once

#include "ScaffNode.h"
#include "Link.h"
#include "Hinge.h"
#include "RodNode.h"
#include "PatchNode.h"

class ScaffLink final : public Structure::Link
{
public:
    ScaffLink(ScaffNode* n1, ScaffNode* n2, Hinge* h = nullptr);
	ScaffLink(ScaffLink& other);
	~ScaffLink();

	Link* clone();

	ScaffNode* fix();
	void draw();

public:
	Hinge* hinge;
};

#define ACTIVE_LINK_TAG "isActiveLink"