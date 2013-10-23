#pragma once

class Edge;

class Node
{
public:
    Node();
    Node(Point &c, std::vector<Vect3d>& axis,
               Vect3d &mScale, std::vector<Edge* > &mEdgeList);
    ~Node();

public:


    //Get the list of adjacent nodes
    std::vector<Node *> getAdjnodes();

private:
    Point center;
	std::vector<Vect3d> axis;
	Vect3d scale;

public:
    std::vector<Edge* > edgeList;

};

