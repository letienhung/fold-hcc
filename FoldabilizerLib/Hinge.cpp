#include "Hinge.h"
#include "Node.h"
#include "CustomDrawObjects.h"

using namespace Geom;

Hinge::Hinge(){}

Hinge::~Hinge(){}

Hinge::Hinge( Node* n1, Node* n2, Point c, Vec3d x, Vector3 y, Vector3 z, double angle_suf, HINGE_TYPE ty, double ext )
{
	this->node1 = n1;
	this->node2 = n2;
	this->center = c;
	this->hX = x.normalized();
	this->hY = y.normalized();
	this->hZ = z.normalized();
	this->maxAngle = angle_suf;
	this->type = ty;
	this->extent = ext;

	// update dihedral frames and angle
	this->state = UNFOLDED;
	this->angle = acos(dot(hX, hY)); 
	this->updateDihedralFrames();

	// node frames
	Frame node1_frame = node1->mBox.getFrame();
	Frame node2_frame = node2->mBox.getFrame();

	// create link record in node frames
	hinge_record1 = createLinkRecord(node1->mBox);
	hinge_record2 = createLinkRecord(node2->mBox);

	// create node records in dihedral frames
	node1_record = createNodeRecord(node1_frame, zxFrame);
	node2_record = createNodeRecord(node2_frame, zyFrame);

	// hinge scale
	this->scale = 0.1;
	this->highlighted = false;
}


Hinge::HingeRecord Hinge::createLinkRecord( Box& node_box )
{
	HingeRecord lr;
	lr.c_box = node_box.getCoordinates(center);

	Frame f = node_box.getFrame();
	lr.c = f.getCoordinates(center);
	lr.cpa = f.getCoordinates(center + hZ);
	lr.cpv1 = f.getCoordinates(center + hX);
	lr.cpv2 = f.getCoordinates(center + hY);

	return lr;
}

Hinge::NodeRecord Hinge::createNodeRecord( Frame node_frame, Frame dl_frame )
{
	NodeRecord nr;
	nr.c = dl_frame.getCoordinates(node_frame.c);
	nr.cpr = dl_frame.getCoordinates(node_frame.r + node_frame.c);
	nr.cps = dl_frame.getCoordinates(node_frame.s + node_frame.c);
	nr.cpt = dl_frame.getCoordinates(node_frame.t + node_frame.c);

	return nr;
}

bool Hinge::fix()
{
	// if both nodes are fixed, treat node2 as free
	// distinguish between fixed and free
	Node *fixed_node, *free_node;
	HingeRecord fixed_hr, free_hr;
	NodeRecord free_nr;
	if (node1->isFixed)	{
		fixed_node = node1; free_node = node2;
		fixed_hr = hinge_record1; free_hr = hinge_record2;
		free_nr = node2_record;
	}else{
		fixed_node = node2; free_node = node1;
		fixed_hr = hinge_record2; free_hr = hinge_record1;
		free_nr = node1_record;
	}

	// fix hinge position and orientation from fixed node
	this->recoverLink(fixed_hr, fixed_node->mBox);

	// fix hinge angle
	this->updateDihedralVectors(node1->isFixed);

	// fix dihedral frames
	this->updateDihedralFrames();

	// fix the free node if it is truly free
	if (free_node->isFixed)
	{
		this->highlighted = false;
		return false; 
	}
	else
	{
		// step 1: fix the frame
		Frame free_dhf = (node1->isFixed)? zyFrame : zxFrame;
		Frame free_nf = this->recoverNodeFrame(free_nr, free_dhf);
		free_node->mBox.setFrame( free_nf ); 
		// step 2: snap two nodes
		Vector3 hc_free = free_node->mBox.getPosition(free_hr.c_box);
		free_node->translate(this->center - hc_free);
		free_node->isFixed = true;

		this->highlighted = true;
		return true;
	}
}

void Hinge::recoverLink( HingeRecord lr, Box& node_box )
{
	this->center = node_box.getPosition(lr.c_box);

	Frame f = node_box.getFrame();
	Vector3 c	= f.getPosition(lr.c);
	this->hZ	= f.getPosition(lr.cpa)  - c;
	this->hX	= f.getPosition(lr.cpv1) - c;
	this->hY	= f.getPosition(lr.cpv2) - c;
}

Frame Hinge::recoverNodeFrame( NodeRecord nr, Frame dl_frame )
{
	Frame node_frame;
	node_frame.c = dl_frame.getPosition(nr.c);
	node_frame.r = dl_frame.getPosition(nr.cpr) - node_frame.c;
	node_frame.s = dl_frame.getPosition(nr.cps) - node_frame.c;
	node_frame.t = dl_frame.getPosition(nr.cpt) - node_frame.c;

	return node_frame;
}

void Hinge::updateDihedralFrames()
{
	zxFrame = Frame(center, hZ, hX, cross(hZ, hX));
	zyFrame = Frame(center, hZ, hY, cross(hZ, hY));
}

void Hinge::updateDihedralVectors( bool isV1Fixed )
{
	if (isV1Fixed)
	{
		Vector3 proj = cross(this->hZ, hX);
		hY = (cos(angle) * hX + sin(angle) * proj).normalized();
	}
	else
	{
		Vector3 proj = cross(hY, this->hZ);
		hX = (cos(angle) * hY + sin(angle) * proj).normalized();
	}
}


void Hinge::draw()
{
	FrameSoup fs(this->scale);
	fs.addFrame( this->hX, this->hY, this->hZ, this->center);
	fs.draw();

	if (highlighted)
	{
		PointSoup ps(16.0);
		ps.addPoint(center, Qt::red);
		ps.draw();
	}
}

void Hinge::setState( int s )
{
	switch(s)
	{
	case FOLDED:
		this->angle = 0;
		this->state = FOLDED;
		break;
	case UNFOLDED:
		this->angle = maxAngle;
		this->state = UNFOLDED;
		break;
	case HALF_FOLDED:
		this->state = HALF_FOLDED;
		break;
	default:
		break;
	}
}

SurfaceMesh::Vector3 Hinge::getDihedralDirec( Node* n )
{
	if (node1 == n) 
		return this->hX;

	if (node2 == n) 
		return this->hY;

	return Vector3(0);
}

