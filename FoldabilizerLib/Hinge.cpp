#include "Hinge.h"
#include "Node.h"
#include "CustomDrawObjects.h"


Hinge::Hinge(){}

Hinge::~Hinge(){}

Hinge::Hinge( Node* n1, Node* n2, Point c, Vec3d x, Vector3 y, Vector3 z, double angle_suf )
{
	this->node1 = n1;
	this->node2 = n2;
	this->center = c;
	this->hX = x.normalized();
	this->hY = y.normalized();
	this->hZ = z.normalized();
	this->maxAngle = angle_suf;

	// update dihedral frames and angle
	this->angle = acos(dot(hX, hY)); 
	this->updateDihedralFrames();

	// node frames
	Frame node1_frame = node1->getFrame();
	Frame node2_frame = node2->getFrame();

	// create link record in node frames
	hinge_record1 = createLinkRecord(node1->mBox);
	hinge_record2 = createLinkRecord(node2->mBox);

	// create node records in dihedral frames
	node1_record = createNodeRecord(node1_frame, zxFrame);
	node2_record = createNodeRecord(node2_frame, zyFrame);

	// hinge scale
	this->scale = 0.1;
}


Hinge::HingeRecord Hinge::createLinkRecord( Box& node_box )
{
	HingeRecord lr;
	lr.uc = node_box.getUniformCoordinates(center);

	lr.c = node_box.getCoordinates(center);
	lr.cpa = node_box.getCoordinates(center + hZ);
	lr.cpv1 = node_box.getCoordinates(center + hX);
	lr.cpv2 = node_box.getCoordinates(center + hY);

	return lr;
}

Hinge::NodeRecord Hinge::createNodeRecord( Frame node_frame, Frame dl_frame )
{
	NodeRecord nr;
	nr.c = dl_frame.coordinates(node_frame.c);
	nr.cpr = dl_frame.coordinates(node_frame.r + node_frame.c);
	nr.cps = dl_frame.coordinates(node_frame.s + node_frame.c);
	nr.cpt = dl_frame.coordinates(node_frame.t + node_frame.c);

	return nr;
}

bool Hinge::fix()
{
	// cases where no fix is needed
	if (node1->isFixed && node2->isFixed) return false; 

	// distinguish between fixed and free
	Node *fixed_node, *free_node;
	HingeRecord fixed_lr, free_lr;
	NodeRecord free_nr;
	if (node1->isFixed)	{
		fixed_node = node1; free_node = node2;
		fixed_lr = hinge_record1; free_lr = hinge_record2;
		free_nr = node2_record;
	}else{
		fixed_node = node2; free_node = node1;
		fixed_lr = hinge_record2; free_lr = hinge_record1;
		free_nr = node1_record;
	}

	// fix hinge position and orientation from fixed node
	this->recoverLink(fixed_lr, fixed_node->mBox);

	// fix hinge angle
	updateDihedralVectors(node1->isFixed);

	// fix dihedral frames
	updateDihedralFrames();

	// fix node on the other end
	// step 1: fix the original box
	Frame free_dlf = (node1->isFixed)? zyFrame : zxFrame;
	free_node->setFrame( this->recoverNodeFrame(free_nr, free_dlf) ); 
	// step 2: the current box differs by a scale factor, which can be fixed by a translation
	free_node->fix();
	Vector3 link_center_on_free_node = free_node->mBox.getUniformPosition(free_lr.uc);
	free_node->mBox.Center += center - link_center_on_free_node;

	// mark the tag
	free_node->isFixed = true;
	return true;
}

void Hinge::recoverLink( HingeRecord lr, Box& node_box )
{
	this->center = node_box.getUniformPosition(lr.uc);

	Vector3 c	= node_box.getPosition(lr.c);
	this->hZ	= node_box.getPosition(lr.cpa)  - c;
	this->hX	= node_box.getPosition(lr.cpv1) - c;
	this->hY	= node_box.getPosition(lr.cpv2) - c;
}

Frame Hinge::recoverNodeFrame( NodeRecord nr, Frame dl_frame )
{
	Frame node_frame;
	node_frame.c = dl_frame.position(nr.c);
	node_frame.r = dl_frame.position(nr.cpr) - node_frame.c;
	node_frame.s = dl_frame.position(nr.cps) - node_frame.c;
	node_frame.t = dl_frame.position(nr.cpt) - node_frame.c;

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


void Hinge::draw( bool highlight )
{
	FrameSoup fs(this->scale);
	fs.addFrame( this->hX, this->hY, this->hZ, this->center);
	fs.draw();

	if (highlight)
	{
		PointSoup ps(16.0);
		ps.addPoint(center, Qt::red);
		ps.draw();
	}
}

