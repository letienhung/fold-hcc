#include "Hinge.h"
#include "CustomDrawObjects.h"

Hinge::Hinge()
{
}

Hinge::Hinge( FdNode* n1, FdNode* n2, Point o, Vec3d x, Vector3 y, Vector3 z, double extent )
{
	node1 = n1;
	node2 = n2;

	Origin = o;
	hX = x.normalized();
	hY = y.normalized();
	hZ = z.normalized();
	zExtent = extent;

	// update dihedral frames and angle
	state = UNFOLDED;
	double dotProd = dot(hX, hY);
	angle = acos(RANGED(-1, dotProd, 1));

	qDebug() << "dotProd = " << dotProd << ", angle = " << angle;


	maxAngle = angle;
	updateDihedralFrames();

	// node frames
	Geom::Frame node1_frame = node1->mBox.getFrame();
	Geom::Frame node2_frame = node2->mBox.getFrame();

	// create link record in node frames
	hinge_record1 = createLinkRecord(node1->mBox);
	hinge_record2 = createLinkRecord(node2->mBox);

	// create node records in dihedral frames
	node1_record = createNodeRecord(node1_frame, zxFrame);
	node2_record = createNodeRecord(node2_frame, zyFrame);
}


Hinge::HingeRecord Hinge::createLinkRecord( Geom::Box& node_box )
{
	HingeRecord lr;
	lr.c_box = node_box.getCoordinates(Origin);

	Geom::Frame f = node_box.getFrame();
	lr.c = f.getCoordinates(Origin);
	lr.cpa = f.getCoordinates(Origin + hZ);
	lr.cpv1 = f.getCoordinates(Origin + hX);
	lr.cpv2 = f.getCoordinates(Origin + hY);

	return lr;
}

Hinge::NodeRecord Hinge::createNodeRecord( Geom::Frame node_frame, Geom::Frame dl_frame )
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
	FdNode *fixed_node, *free_node;
	HingeRecord fixed_hr, free_hr;
	NodeRecord free_nr;
	if (node1->properties["fixed"].toBool())
	{
		fixed_node = node1; free_node = node2;
		fixed_hr = hinge_record1; free_hr = hinge_record2;
		free_nr = node2_record;
	}else
	{
		fixed_node = node2; free_node = node1;
		fixed_hr = hinge_record2; free_hr = hinge_record1;
		free_nr = node1_record;
	}

	// fix hinge position and orientation from fixed node
	recoverLink(fixed_hr, fixed_node->mBox);

	// fix hinge angle
	updateDihedralVectors(node1->properties["fixed"].toBool());

	// fix dihedral frames
	updateDihedralFrames();

	// fix the free node if it is truly free
	if (free_node->properties["fixed"].toBool())
	{
		return false; 
	}
	else
	{
		// step 1: fix the frame
		Geom::Frame free_dhf = (node1->properties["fixed"].toBool())? zyFrame : zxFrame;
		Geom::Frame free_nf = this->recoverNodeFrame(free_nr, free_dhf);
		free_node->mBox.setFrame( free_nf ); 

		// step 2: snap two nodes
		Vector3 hc_free = free_node->mBox.getPosition(free_hr.c_box);
		free_node->mBox.translate(Origin - hc_free);
		free_node->properties["fixed"] = true;

		// step 3: update scaffold
		free_node->createScaffold();

		return true;
	}
}

void Hinge::recoverLink( HingeRecord lr, Geom::Box& node_box )
{
	this->Origin = node_box.getPosition(lr.c_box);

	Geom::Frame f = node_box.getFrame();
	Vector3 c	= f.getPosition(lr.c);
	this->hZ	= f.getPosition(lr.cpa)  - c;
	this->hX	= f.getPosition(lr.cpv1) - c;
	this->hY	= f.getPosition(lr.cpv2) - c;
}

Geom::Frame Hinge::recoverNodeFrame( NodeRecord nr, Geom::Frame dl_frame )
{
	Geom::Frame node_frame;
	node_frame.c = dl_frame.getPosition(nr.c);
	node_frame.r = dl_frame.getPosition(nr.cpr) - node_frame.c;
	node_frame.s = dl_frame.getPosition(nr.cps) - node_frame.c;
	node_frame.t = dl_frame.getPosition(nr.cpt) - node_frame.c;

	return node_frame;
}

void Hinge::updateDihedralFrames()
{
	zxFrame = Geom::Frame(Origin, hZ, hX, cross(hZ, hX));
	zyFrame = Geom::Frame(Origin, hZ, hY, cross(hZ, hY));
}

void Hinge::updateDihedralVectors( bool hXFixed )
{
	if (hXFixed)
	{
		Vector3 crossZX = cross(hZ, hX);
		hY = (cos(angle) * hX + sin(angle) * crossZX).normalized();
	}
	else
	{
		Vector3 crossYZ = cross(hY, hZ);
		hX = (cos(angle) * hY + sin(angle) * crossYZ).normalized();
	}
}


void Hinge::draw()
{
	FrameSoup fs(zExtent);
	fs.addFrame( hX, hY, hZ, Origin);
	fs.draw();
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

Vector3 Hinge::getDihedralDirec( FdNode* n )
{
	if (node1 == n) 
		return hX;

	if (node2 == n) 
		return hY;

	return Vector3(0);
}

void Hinge::setAngleByTime( double t )
{
	angle = maxAngle * (1 - t);
}

