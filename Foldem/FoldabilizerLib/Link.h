#pragma once

#include "FoldabilizerLibGlobal.h"
#include "Frame.h"

class Node;

// v1   axis
// ^   /
// |  /
// | /
// |/
// +--------> v2   
// NOTE: dot(v1, axis) = 0, dot(v2, axis) = 0
// A hinge codes a dihedral angle: two planes intersect at the hinge axis
// plane_i is spanned by axis and v; and by two axes of node_i
// such that at least one axis of node_i must be perpendicular to axis, denoted as vp_i
// v_i = cross(axis, vp_i)

class Link
{
public:
	Node  *node1, *node2;
	Point center;				// contact point
	Vec3d axis;					// axis direction
	Vec3d v1, v2;				// dihedral directions
	Frame frame1, frame2;		// dihedral frames

	struct LinkRecord{
		Vec3d c, a, v1, v2;
	}record1, record2;			// link records in incident nodes

    Link(Node* n1, Node* n2, Point c, Vec3d a);
    ~Link(){};

public:
	bool  hasNode(QString nodeID);
	Node* getNode(QString nodeID);
	Node* otherNode(QString nodeID);

	void draw();
};

