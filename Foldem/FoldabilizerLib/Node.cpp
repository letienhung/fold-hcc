#include "Node.h"
#include <QDebug>

Node::Node(Box &b, QVector<Edge* > &eList)
{
	mBox = b;
    edgeList = eList;
}

Node::~Node()
{
    int mSize = edgeList.size();
    if(mSize){
		for(int i = 0; i < mSize; i++)
			if(edgeList[i])
				delete edgeList[i];   
        edgeList.clear();
	}
}

QVector<Node *> Node::getAdjnodes()
{
    //TODO
}
