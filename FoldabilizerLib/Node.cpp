#include "Node.h"
#include "../CustomDrawObjects.h"

#include <QDebug>

Node::Node(Box b, QString id)
{
	mBox = b; 
	mID = id;
	mColor = qRandomColor(); mColor.setAlphaF(0.5);

	isFixed = false;

	originalExtent = mBox.Extent;
	scaleFactor = Vector3(1,1,1);

	isHighlight = false;
}

Node::~Node()
{
}

void Node::draw()
{
	PolygonSoup ps;
	foreach(QVector<Point> f, mBox.getFacePoints()) ps.addPoly(f, mColor);

	// draw faces
	ps.drawQuads(true);

	// highlight wireframes
	if (isHighlight) 
	{
		QColor c = isFixed ? Qt::white : Qt::red;
		ps.drawWireframes(2.0, c);	
	}
}




// return a direction that is perpendicular to hing_axis on the dihedral plane
SurfaceMesh::Vec3d Node::dihedralDirection( Vec3d hinge_pos, Vec3d hinge_axis )
{
	// hinge is along one axis, thus perpendicular to two others which span the dihedral plane
	// dd is the axis with larger extent
	for(int i = 0; i < 3; i++){
		if (cross(mBox.Axis[i], hinge_axis).norm() < 0.1)
		{
			Vec3d dd = (mBox.Extent[(i+1)%3] >= mBox.Extent[(i+2)%3]) ?
						mBox.Axis[(i+1)%3] : mBox.Axis[(i+2)%3];
			Vec3d h2n = (mBox.Center - hinge_pos).normalized();

			if (dot(dd, h2n) < 0) dd *= -1;
			return dd;
		}
	}

	// hinge is perpendicular to one node axis, which is also perpendicular to the dihedral plane
	// dd is the cross product of hinge and that node axis
	foreach(Vector3 v, mBox.Axis){
		if (abs(dot(v, hinge_axis)) < 0.1)
			return cross(v, hinge_axis);
	}

	return Vec3d();
}

Frame Node::getFrame()                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
{
	return mBox.getFrame();
}

void Node::setFrame( Frame f )
{
	mBox.setFrame(f);
}

void Node::translate( Vector3 t )
{
	mBox.Center += t;
}



void Node::fix()
{
	for (int i = 0 ; i < 3; i++)
	{
		mBox.Extent[i] = scaleFactor[i] * originalExtent[i];
	}
	
}

double Node::getVolume()
{
	Vector3 e = 2 * mBox.Extent;
	return e[0] * e[1] * e[2];
}

