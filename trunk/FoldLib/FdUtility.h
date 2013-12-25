#pragma once

#include "UtilityGlobal.h"
#include "FdNode.h"
#include "PatchNode.h"
#include "Segment.h"

Geom::Segment getDistSegment( FdNode* n1, FdNode* n2 );
double getDistance( FdNode* n1, FdNode* n2 );
double getDistance( FdNode* n, QVector<FdNode*> nset);

FdNodeArray2D clusterNodes( QVector<FdNode*> nodes, double disThr );

StrArray2D getIds(FdNodeArray2D nodeArray);

QVector<Geom::Segment> detectHinges(FdNode* part, PatchNode* panel);