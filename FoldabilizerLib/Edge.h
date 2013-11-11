#pragma once

#include "FoldabilizerLibGlobal.h"

class Node;

class Edge
{
public:
    Edge();
    Edge(Point &f, Point &t, Node* n1, Node* n2);
    ~Edge(){}

public:
	//Hinge score
	double score;
	//Is folded(true) or not(false)
	bool isFolded;
    //Angle between two cuboids
	double angle;
    //Position to attach the hinge
	Point from;
	Point to;

public:
    void setCuboid(Node* n1, Node* n2);

    Node* getNode1();
    Node* getNode2();

	//Visualize Hinge as a line
	void draw();

	//Calculate angle 
	double calAngle();

	//Calculate folding score
	double calScore();
						  
private:
    Node *node1;
    Node *node2;
};
