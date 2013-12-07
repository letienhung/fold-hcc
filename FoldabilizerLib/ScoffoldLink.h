#pragma once

#include "ScoffoldNode.h"
#include "Link.h"

class ScoffoldLink : public Structure::Link
{
public:
    ScoffoldLink(ScoffoldNode* n1, ScoffoldNode* n2);
};


