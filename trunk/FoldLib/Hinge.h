#pragma once
#include "UtilityGlobal.h"
#include "Frame.h"
#include "Box.h"

#include "FdNode.h"

//			hX   hZ
//			^   /
//			|  /
//			| /
//			|/
//  Origin	+--------> hY  

// NOTE: hZ = cross(hX, hY), but hX and hY don't form a right angle


class Hinge
{
public:
	Hinge(FdNode* n1, FdNode* n2, Vector3 c, Vector3 x, Vector3 y, Vector3 z, double extent);

	FdNode	*node1, *node2;

	Vector3	Origin;						// contact point
	Vector3	hX, hY, hZ;					// axis and dihedral directions
	double zExtent;

	double	angle, maxAngle;			// angle <v1, v2> in radians
	Geom::Frame	zxFrame, zyFrame;		// dihedral frames

	enum{
		FOLDED, UNFOLDED, HALF_FOLDED
	} state;							// folding state

	struct HingeRecord{			
		Vector3 c_box;					// relative position in box
		Vector3 c, cpa, cpv1, cpv2;		// absolute positions in box-frame to recover vectors
	}hinge_record1, hinge_record2;

	struct NodeRecord{					// node records in dihedral frames
		Vector3 c, cpr, cps, cpt;
	}node1_record, node2_record;

public:
	// restore the configuration of self and two incident nodes
	bool fix();

	// getters
	Vector3 getDihedralDirec(FdNode* n);

	// state
	void setState(int s);

	// visualization
	bool highlighted;
	double scale;
	void draw();

private:
	void updateDihedralVectors(bool hXFixed);
	void updateDihedralFrames();
	HingeRecord createLinkRecord(Geom::Box& node_box);
	NodeRecord createNodeRecord(Geom::Frame node_frame, Geom::Frame dl_frame);
	void recoverLink(HingeRecord lr, Geom::Box& node_box);
	Geom::Frame recoverNodeFrame(NodeRecord nr, Geom::Frame dl_frame);
};