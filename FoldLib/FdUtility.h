#pragma once

#include "UtilityGlobal.h"
#include "FdNode.h"
#include "Segment.h"

Geom::Segment getDistSegment( FdNode* n1, FdNode* n2 );
double getDistance( FdNode* n1, FdNode* n2 );
double getDistance( FdNode* n, QVector<FdNode*> nset);

FdNodeArray2D clusterNodes( QVector<FdNode*> nodes, double disThr );
