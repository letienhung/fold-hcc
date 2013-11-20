#pragma once
#include "FoldabilizerLibGlobal.h"
#include "Frame.h"
#include "Box.h"

class Node;

// hX   hZ
// ^   /
// |  /
// | /
// |/
// +--------> hY   
// NOTE: hZ is perpendicular to both hX and hY
// A hinge represents a dihedral angle: two planes intersect at the hinge axis


class Hinge
{
public:
    Hinge();
	~Hinge();
	Hinge(Node* n1, Node* n2, Point c, Vec3d x, Vector3 y, Vector3 z, double angle_suf);

	Node	*node1, *node2;
	Point	center;				// contact point
	Vector3	hX, hY, hZ;			// axis and dihedral directions

	double	angle;				// angle <v1, v2> in radians: [0, maxAngle]
	double	maxAngle;			// pi/2 or pi
	Frame	zxFrame, zyFrame;	// dihedral frames

	struct HingeRecord{			// hinge records in node frames
		Vec3d uc, c, cpa, cpv1, cpv2;
	}hinge_record1, hinge_record2;

	struct NodeRecord{			// node records in dihedral frames
		Vec3d c, cpr, cps, cpt;
	}node1_record, node2_record;

	bool fix();

	double scale;
	void draw(bool highlight);

private:
	void updateDihedralVectors(bool fix_v1);
	void updateDihedralFrames();
	HingeRecord createLinkRecord(Box& node_box);
	NodeRecord createNodeRecord(Frame node_frame, Frame dl_frame);
	void recoverLink(HingeRecord lr, Box& node_box);
	Frame recoverNodeFrame(NodeRecord nr, Frame dl_frame);
};
