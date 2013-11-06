#include "Link.h"
#include "Node.h"
#include "../CustomDrawObjects.h"

Link::Link(Node* n1, Node* n2, Point c, Vec3d a)
	:node1(n1), node2(n2), center(c), axis(a)
{
	// dihedral directions
	this->v1 = node1->dihedralDirection(center, axis).normalized();
	this->v2 = node2->dihedralDirection(center, axis).normalized();
	if (dot(cross(v1, v2), axis) < 0) axis *= -1;

	// update dihedral frames and angle
	this->updateDihedralFrames();
	angle = acos(dot(v1, v2)); angle_suf = angle;

	// get node frames
	Frame node1_frame = node1->getFrame();
	Frame node2_frame = node2->getFrame();

	// create link record in node frames
	link_record1 = createLinkRecord(node1_frame);
	link_record2 = createLinkRecord(node2_frame);

	// create node records in dihedral frames
	node1_record = createNodeRecord(node1_frame, frame1);
	node2_record = createNodeRecord(node2_frame, frame2);
}

void Link::draw()
{
	FrameSoup fs(1);
	fs.addFrame(axis, v1, v2, center);
	fs.draw();
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

Link::LinkRecord Link::createLinkRecord( Frame node_frame )
{
	LinkRecord lr;
	lr.c = node_frame.coordinates(center);
	lr.cpa = node_frame.coordinates(center + axis);
	lr.cpv1 = node_frame.coordinates(center + v1);
	lr.cpv2 = node_frame.coordinates(center + v2);

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

void Link::fix()
{
	// special case: both nodes are fixed
	if (node1->isFixed && node2->isFixed) return; 

	// distinguish between fixed and free
	Node *fixed_node, *free_node;
	LinkRecord fixed_lr;
	NodeRecord free_nr;
	if (node1->isFixed)	{
		fixed_node = node1; free_node = node2;
		fixed_lr = link_record1;
		free_nr = node2_record;
	}else{
		fixed_node = node2; free_node = node1;
		fixed_lr = link_record2;
		free_nr = node1_record;
	}

	// fix hinge position and orientation from fixed node
	this->recoverLinkFromRecord(fixed_lr, fixed_node->getFrame());

	// fix hinge angle
	updateDihedralVectors(node1->isFixed);

	// fix dihedral frames
	updateDihedralFrames();

	// fix node on the other end
	Frame free_dlf = (node1->isFixed)? frame2 : frame1;
	free_node->setFrame( this->recoverNodeFrameFromRecord(free_nr, free_dlf) );
}

void Link::changeAngle()
{
	angle -= 0.1;
	if (angle < 0) angle = angle_suf;
}

void Link::recoverLinkFromRecord( LinkRecord lr, Frame node_frame )
{
	this->center = node_frame.position(lr.c);
	this->axis = node_frame.position(lr.cpa) - this->center;
	this->v1 = node_frame.position(lr.cpv1) - this->center;
	this->v2 = node_frame.position(lr.cpv2) - this->center;
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
	Vec3d fixed_v = isV1Fixed ? v1 : v2;
	Vec3d proj = cross(this->axis, fixed_v);

	Vec3d free_v = cos(angle) * fixed_v + sin(angle) * proj;
	free_v.normalize();

	if (isV1Fixed) v2 = free_v;
	else v1 = free_v;
}
