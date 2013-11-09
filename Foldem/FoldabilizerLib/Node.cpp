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
}

Node::~Node()
{
}

void Node::draw()
{
	PolygonSoup ps;
	foreach(QVector<Point> f, getBoxFaces()) ps.addPoly(f, mColor);

	// draw faces
	ps.drawQuads(true);

	// highlight wireframes
	if (isFixed) 
		ps.drawWireframes(2.0, Qt::white);	
}

QVector<Point> Node::getBoxConners()
{
	QVector<Point> pnts(8);

	// Create right-hand system
	if ( dot(cross(mBox.Axis[0], mBox.Axis[1]), mBox.Axis[2]) < 0 ) 
		mBox.Axis[2]  = -mBox.Axis[2];

	std::vector<Vec3d> Axis;
	for (int i=0;i<3;i++)
		Axis.push_back( 2 * mBox.Extent[i] * mBox.Axis[i]);

	pnts[0] = mBox.Center - 0.5*Axis[0] - 0.5*Axis[1] + 0.5*Axis[2];
	pnts[1] = pnts[0] + Axis[0];
	pnts[2] = pnts[1] - Axis[2];
	pnts[3] = pnts[2] - Axis[0];

	pnts[4] = pnts[0] + Axis[1];
	pnts[5] = pnts[1] + Axis[1];
	pnts[6] = pnts[2] + Axis[1];
	pnts[7] = pnts[3] + Axis[1];

	return pnts;
}

QVector< QVector<Point> > Node::getBoxFaces()
{
	QVector< QVector<Point> > faces(6);
	QVector<Point> pnts = getBoxConners();

	for (int i = 0; i < 6; i++)	{
		for (int j = 0; j < 4; j++)	{
			faces[i].push_back( pnts[ cubeIds[i][j] ] );
		}
	}	

	return faces;
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
	return Frame(mBox.Center, mBox.Axis[0], mBox.Axis[1], mBox.Axis[2]);
}

void Node::translate( Vector3 t )
{
	mBox.Center += t;
}

void Node::setFrame( Frame f )
{
	mBox.Center = f.c;
	mBox.Axis[0] = f.r;
	mBox.Axis[1] = f.s;
	mBox.Axis[2] = f.t;
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

