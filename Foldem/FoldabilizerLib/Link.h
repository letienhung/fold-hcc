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

	double angle;				// angle <v1, v2> in radians: [0, angle_suf]
	double angle_suf;			// 0 < angle_suf < pi
	Frame frame1, frame2;		// dihedral frames

	struct LinkRecord{
		Vec3d c, cpa, cpv1, cpv2;
	}link_record1, link_record2;			// link records in node frames

	struct NodeRecord{
		Vec3d c, cpr, cps, cpt;
	}node1_record, node2_record;			// node records in dihedral frames

    Link(Node* n1, Node* n2, Point c, Vec3d a);
    ~Link(){};

private:
	void updateDihedralVectors(bool fix_v1);
	void updateDihedralFrames();
	LinkRecord createLinkRecord(Frame node_frame);
	NodeRecord createNodeRecord(Frame node_frame, Frame dl_frame);
	void recoverLinkFromRecord(LinkRecord lr, Frame node_frame);
	Frame recoverNodeFrameFromRecord(NodeRecord nr, Frame dl_frame);
	

public:
	bool  hasNode(QString nodeID);
	Node* getNode(QString nodeID);
	Node* otherNode(QString nodeID);

	void changeAngle();
	void fix();

	void draw();
};

