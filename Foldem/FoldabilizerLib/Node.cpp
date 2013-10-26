#include "Node.h"
//#include "Link.h"

#include <QDebug>

Node::Node(Box &b, QString &id)
{
	mBox = b;
	mID = id;
}

Node::~Node()
{
    int mSize = linkList.size();
    if(mSize){
		for(int i = 0; i < mSize; i++)
            if(linkList[i])
                delete linkList[i];
        linkList.clear();
	}
}

QVector<Node *> Node::getAdjnodes()
{
	QVector<Node *> adjNodes;
	//TODO
	return adjNodes;
}
