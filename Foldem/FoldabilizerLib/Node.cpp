#include "Node.h"
#include <QDebug>

Node::Node()
{
	center = Point(0,0,0);
	scale = Eigen::Vector3d(0,0,0);

}

Node::Node(Point &mCenter, std::vector<Vect3d>& mAxis,
                       Vect3d &mScale, std::vector<Edge* > &mEdgeList)
{
	center = mCenter;
	axis = mAxis;
	scale = mScale;
    edgeList = mEdgeList;
}

Node::~Node()
{
	axis.clear();
	
    int mSize = edgeList.size();
    if(mSize){
		for(int i = 0; i < mSize; i++)
            delete edgeList[i];
	    
        edgeList.clear();
	}
}

std::vector<Node *> Node::getAdjnodes()
{
    //TODO
}
