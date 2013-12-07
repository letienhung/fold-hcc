#pragma once

#include "FdNode.h"
#include "Link.h"

class FdLink : public Structure::Link
{
public:
    FdLink(FdNode* n1, FdNode* n2);
};


