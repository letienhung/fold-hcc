#pragma once

#include "FdNode.h"
#include "Link.h"
#include "Hinge.h"
#include "RodNode.h"
#include "PatchNode.h"

class FdLink : public Structure::Link
{
public:
    FdLink(FdNode* n1, FdNode* n2, bool detect = true);
	FdLink(FdLink& other);
	~FdLink();

	virtual Link* clone();

	void detectHinges();
	void detectHinges(PatchNode* base, PatchNode* branch);
	void detectHinges(RodNode* rnode, PatchNode* pnode);

	virtual void draw();

public:
	QVector<Hinge> hinges;
};


