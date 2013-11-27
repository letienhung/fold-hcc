#pragma once
#include "UtilityGlobal.h"
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
	Vector3 P0, P1;				// two end points of an edge hinge 

	double	angle;				// angle <v1, v2> in radians: [0, maxAngle]
	double	maxAngle;			// pi/2 or pi
	Geom::Frame	zxFrame, zyFrame;	// dihedral frames

	enum{
		FOLDED, UNFOLDED, HALF_FOLDED
	} state;					// folding state

	struct HingeRecord{			// hinge records in node frames
		Vec3d uc, c, cpa, cpv1, cpv2;
	}hinge_record1, hinge_record2;

	struct NodeRecord{			// node records in dihedral frames
		Vec3d c, cpr, cps, cpt;
	}node1_record, node2_record;

public:
	bool fix();

	// state
	void setState(int s);

	// visualization
	double scale;
	void draw(bool highlight);

private:
	void updateDihedralVectors(bool fix_v1);
	void updateDihedralFrames();
	HingeRecord createLinkRecord(Geom::Box& node_box);
	NodeRecord createNodeRecord(Geom::Frame node_frame, Geom::Frame dl_frame);
	void recoverLink(HingeRecord lr, Geom::Box& node_box);
	Geom::Frame recoverNodeFrame(NodeRecord nr, Geom::Frame dl_frame);
};
