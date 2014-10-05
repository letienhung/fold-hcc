#include "Hinge.h"
#include "CustomDrawObjects.h"

Hinge::Hinge()
{
}

Hinge::Hinge( ScaffNode* n1, ScaffNode* n2, Point o, Vec3d x, Vector3 y, Vector3 z, double extent )
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
	maxAngle = angle;
	updateDihedralFrames();

	// node frames
	Geom::Frame node1_frame = node1->mBox.getFrame();
	Geom::Frame node2_frame = node2->mBox.getFrame();

	// create link record in node frames
	hinge_record1 = createHingeRecord(node1->mBox);
	hinge_record2 = createHingeRecord(node2->mBox);

	// create node records in dihedral frames
	n1_frecord = zxFrame.encodeFrame(node1_frame);
	n2_frecord = zyFrame.encodeFrame(node2_frame);
}


Hinge::HingeRecordInBox Hinge::createHingeRecord( Geom::Box& node_box )
{
	HingeRecordInBox lr;
	lr.c_box = node_box.getCoordinates(Origin);

	Geom::Frame f = node_box.getFrame();
	lr.c = f.getCoordinates(Origin);
	lr.cpa = f.getCoordinates(Origin + hZ);
	lr.cpv1 = f.getCoordinates(Origin + hX);
	lr.cpv2 = f.getCoordinates(Origin + hY);

	return lr;
}

ScaffNode* Hinge::fix()
{
	// if both nodes are fixed, treat node2 as free
	// distinguish between fixed and free
	ScaffNode *fixed_node, *free_node;
	HingeRecordInBox fixed_hr, free_hr;
	Geom::Frame::RecordInFrame free_nr;
	if (node1->hasTag(FIXED_NODE_TAG))
	{
		fixed_node = node1; free_node = node2;
		fixed_hr = hinge_record1; free_hr = hinge_record2;
		free_nr = n2_frecord;
	}else
	{
		fixed_node = node2; free_node = node1;
		fixed_hr = hinge_record2; free_hr = hinge_record1;
		free_nr = n1_frecord;
	}

	// skip if free node is not free
	if (free_node->hasTag(FIXED_NODE_TAG))
	{
		return nullptr;
	}

	// fix hinge position and orientation from fixed node
	recoverLink(fixed_hr, fixed_node->mBox);

	// fix hinge angle
	updateDihedralVectors(node1->hasTag(FIXED_NODE_TAG));

	// fix dihedral frames
	updateDihedralFrames();

	// fix the free node
	// step 1: fix the frame
	Geom::Frame free_dhf = (node1->hasTag(FIXED_NODE_TAG))? zyFrame : zxFrame;
	Geom::Frame free_nf = free_dhf.decodeFrame(free_nr);
	free_node->mBox.setFrame( free_nf ); 

	// step 2: snap two nodes
	Vector3 hc_free = free_node->mBox.getPosition(free_hr.c_box);
	free_node->translate(Origin - hc_free);
	free_node->addTag(FIXED_NODE_TAG);

	return free_node;
}

void Hinge::recoverLink( HingeRecordInBox lr, Geom::Box& node_box )
{
	this->Origin = node_box.getPosition(lr.c_box);

	Geom::Frame f = node_box.getFrame();
	Vector3 c	= f.getPosition(lr.c);
	this->hZ	= f.getPosition(lr.cpa)  - c;
	this->hX	= f.getPosition(lr.cpv1) - c;
	this->hY	= f.getPosition(lr.cpv2) - c;
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
		Vector3 Y = cross(hZ, hX);
		hY = (cos(angle) * hX + sin(angle) * Y).normalized();
	}
	else
	{
		Vector3 X = cross(hY, hZ);
		hX = (cos(angle) * hY + sin(angle) * X).normalized();
	}
}


void Hinge::draw()
{
	FrameSoup fs(zExtent);
	fs.addFrame( hX, hY, hZ, Origin);
	//fs.addFrame(zxFrame.r, zxFrame.s, zxFrame.t, zxFrame.c);
	//fs.addFrame(zyFrame.r, zyFrame.s, zyFrame.t, zyFrame.c);
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

Vector3 Hinge::getDihedralDirec( ScaffNode* n )
{
	if (node1 == n) 
		return hX;

	if (node2 == n) 
		return hY;

	return Vector3(0);
}