#include "Link.h"
#include "Node.h"
#include "../CustomDrawObjects.h"

Link::Link(Node* n1, Node* n2, Point c, Vec3d a)
	:node1(n1), node2(n2), center(c), axis(a)
{
	// series id
	this->id = node1->mID + ":" + node2->mID;

	// dihedral directions
	this->v1 = node1->dihedralDirection(center, axis).normalized();
	this->v2 = node2->dihedralDirection(center, axis).normalized();
	double dot_cross_prod = dot(cross(v1, v2), axis);
	if ( abs(dot_cross_prod) < 0.01)
	{
		// the angle is close to \pi
		Vector3 hn1 = node1->mBox.Center - center;
		Vector3 hn2 = node2->mBox.Center - center;
		if (dot(cross(hn1, hn2), axis) > 0) axis *= -1;
	}
	else if (dot_cross_prod < 0) axis *= -1;

	// update dihedral frames and angle
	this->updateDihedralFrames();
	angle = acos(dot(v1, v2)); angle_suf = angle;

	// get node frames
	Frame node1_frame = node1->getFrame();
	Frame node2_frame = node2->getFrame();

	// create link record in node frames
	link_record1 = createLinkRecord(node1->mBox);
	link_record2 = createLinkRecord(node2->mBox);

	// create node records in dihedral frames
	node1_record = createNodeRecord(node1_frame, frame1);
	node2_record = createNodeRecord(node2_frame, frame2);

	// tag
	isFixed = false;
	isBroken = false;
	isNailed = false;
}

void Link::draw()
{
	if (isBroken || isNailed) return;
	
	FrameSoup fs(1.0);
	fs.addFrame(axis, v1, v2, center);
	fs.draw();

	if (isFixed)
	{
		PointSoup ps(15.0);
		ps.addPoint(center, Qt::red);
		ps.draw();
	}
}

bool Link::hasNode( QString nodeID )
{
	return node1->mID == nodeID || node2->mID == nodeID;
}

Node* Link::otherNode( QString nodeID )
{
	return (nodeID == node1->mID)? node2 : node1;
}

Node* Link::getNode( QString nodeID )
{
	return (nodeID == node1->mID)? node1 : node2;
}

Link::LinkRecord Link::createLinkRecord( Box& node_box )
{
	LinkRecord lr;
	lr.uc = node_box.getUniformCoordinates(center);

	lr.c = node_box.getCoordinates(center);
	lr.cpa = node_box.getCoordinates(center + axis);
	lr.cpv1 = node_box.getCoordinates(center + v1);
	lr.cpv2 = node_box.getCoordinates(center + v2);

	return lr;
}



Link::NodeRecord Link::createNodeRecord( Frame node_frame, Frame dl_frame )
{
	NodeRecord nr;
	nr.c = dl_frame.coordinates(node_frame.c);
	nr.cpr = dl_frame.coordinates(node_frame.r + node_frame.c);
	nr.cps = dl_frame.coordinates(node_frame.s + node_frame.c);
	nr.cpt = dl_frame.coordinates(node_frame.t + node_frame.c);

	return nr;
}

bool Link::fix()
{
	// cases where no fix is needed
	if (isBroken || isFixed) return false;
	if (node1->isFixed && node2->isFixed) return false; 

	// distinguish between fixed and free
	Node *fixed_node, *free_node;
	LinkRecord fixed_lr, free_lr;
	NodeRecord free_nr;
	if (node1->isFixed)	{
		fixed_node = node1; free_node = node2;
		fixed_lr = link_record1; free_lr = link_record2;
		free_nr = node2_record;
	}else{
		fixed_node = node2; free_node = node1;
		fixed_lr = link_record2; free_lr = link_record1;
		free_nr = node1_record;
	}

	// fix hinge position and orientation from fixed node
	this->recoverLinkFromRecord(fixed_lr, fixed_node->mBox);

	// fix hinge angle
	updateDihedralVectors(node1->isFixed);

	// fix dihedral frames
	updateDihedralFrames();
	
	// fix node on the other end
	// step 1: fix the original box
	Frame free_dlf = (node1->isFixed)? frame2 : frame1;
	free_node->setFrame( this->recoverNodeFrameFromRecord(free_nr, free_dlf) ); 
	// step 2: the current box differs by a scale factor, which can be fixed by a translation
	free_node->fix();
	Vector3 link_center_on_free_node = free_node->mBox.getUniformPosition(free_lr.uc);
	free_node->mBox.Center += center - link_center_on_free_node;

	// mark the tag
	free_node->isFixed = true;
	this->isFixed = true;
	return true;
}

void Link::changeAngle()
{
	if (isNailed) return;

	angle -= 0.1;
	if (angle < 0) angle = angle_suf;
}


void Link::recoverLinkFromRecord( LinkRecord lr, Box& node_box )
{
	this->center = node_box.getUniformPosition(lr.uc);

	Vector3 c	= node_box.getPosition(lr.c);
	this->axis	= node_box.getPosition(lr.cpa)  - c;
	this->v1	= node_box.getPosition(lr.cpv1) - c;
	this->v2	= node_box.getPosition(lr.cpv2) - c;
}

Frame Link::recoverNodeFrameFromRecord( NodeRecord nr, Frame dl_frame )
{
	Frame node_frame;
	node_frame.c = dl_frame.position(nr.c);
	node_frame.r = dl_frame.position(nr.cpr) - node_frame.c;
	node_frame.s = dl_frame.position(nr.cps) - node_frame.c;
	node_frame.t = dl_frame.position(nr.cpt) - node_frame.c;

	return node_frame;
}

void Link::updateDihedralFrames()
{
	frame1 = Frame(center, axis, v1, cross(axis, v1));
	frame2 = Frame(center, axis, v2, cross(axis, v2));
}

void Link::updateDihedralVectors( bool isV1Fixed )
{
	if (isV1Fixed)
	{
		Vector3 proj = cross(this->axis, v1);
		v2 = (cos(angle) * v1 + sin(angle) * proj).normalized();
	}
	else
	{
		Vector3 proj = cross(v2, this->axis);
		v1 = (cos(angle) * v2 + sin(angle) * proj).normalized();
	}
}
