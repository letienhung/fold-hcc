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

QVector<Geom::Segment> detectHingeSegments(FdNode* part, PatchNode* panel);

double getLocalTime(double globalT, double localStart, double localEnd);
QVector<double> getEvenDivision(int n, double start = 0, double end = 1);