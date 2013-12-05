#pragma once

#include "UtilityGlobal.h"
#include "Node.h"

class NodeSplitter
{
public:
	NodeSplitter(){}

	static QVector<Node*> uniformSplit(Node* node, int axisId, int N);
	static QVector<Node*> split(Node* node, int axisId, QVector<double> t);
};
