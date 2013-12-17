#include "BBox.h"
#include "Numeric.h"

using namespace Geom;

// Face corners
int cubeIds[6][4] = 
{
	1, 2, 6, 5,
	0, 4, 7, 3,
	4, 5, 6, 7,
	0, 3, 2, 1,
	0, 1, 5, 4,
	2, 3, 7, 6
};

BBox::BBox(const Point& c, const QVector<Vector3>& axis, const Vector3& ext)
{
	Center = c;
	Axis = axis;
	Extent = ext;
	
	isSelected = false;

}

BBox::BBox(const BBox &b)
{
	Center = b.Center;
	Axis = b.Axis;
	Extent = b.Extent;

	isSelected = b.isSelected;
	selPlane = b.selPlane;
}

BBox &BBox::operator =(const BBox &b)
{
	Center = b.Center;
	Axis = b.Axis;
	Extent = b.Extent;
	isSelected = b.isSelected;
	selPlane = b.selPlane;

	return *this;
}

BBox::BBox(const Point& c, const Vector3& ext)
{
	Center = c;
	QVector<Vector3> axis(3);
	axis[0] = Vector3(1.0f,0.0f,0.0f);
    axis[1] = Vector3(0.0f,1.0f,0.0f);
    axis[2] = Vector3(0.0f,0.0f,1.0f);
	Axis = axis;
	Extent = ext;
	isSelected = false;
}

Vector3 BBox::getCoordinates( Vector3 p )
{
	Vector3 coord;
	p = p - Center;
	for (int i = 0; i < 3; i++)
		coord[i] = dot(p, Axis[i]);

	return coord;
}

Vector3 BBox::getPosition( Vector3 coord )
{
	Vector3 pos = Center;
	for (int i = 0; i < 3; i++)
		pos += coord[i] * Axis[i];

	return pos;
}

Vector3 BBox::getUniformCoordinates( Vector3 p )
{
	Vector3 coord;
	p = p - Center;
	for (int i = 0; i < 3; i++)
		coord[i] = dot(p, Axis[i]) / Extent[i];

	return coord;
}

Vector3 BBox::getUniformPosition( Vector3 coord )
{
	Vector3 pos = Center;
	for (int i = 0; i < 3; i++)
		pos += coord[i] * Extent[i] * Axis[i];

	return pos;
}

QVector<Point> BBox::getBoxCorners()
{
	QVector<Point> pnts(8);

	Vector3 crossAxis = cross(Axis[0], Axis[1]);
	// Create right-hand system
	if ( dot(crossAxis , Axis[2]) < 0 ) 
		Axis[2]  = -Axis[2];

	std::vector<Vec3d> axis;
	for (int i=0;i<3;i++)
		axis.push_back( 2 * Extent[i] * Axis[i]);

	pnts[0] = Center - 0.5*axis[0] - 0.5*axis[1] + 0.5*axis[2];
	pnts[1] = pnts[0] + axis[0];
	pnts[2] = pnts[1] - axis[2];
	pnts[3] = pnts[2] - axis[0];

	pnts[4] = pnts[0] + axis[1];
	pnts[5] = pnts[1] + axis[1];
	pnts[6] = pnts[2] + axis[1];
	pnts[7] = pnts[3] + axis[1];

	return pnts;
}

QVector<Line> BBox::getEdges()
{
	QVector<Point> pnts = getBoxCorners();
	QVector<Line> lns(12);

	lns[0] = Line(pnts[0], pnts[1]-pnts[0]);
	lns[1] = Line(pnts[1], pnts[2]-pnts[1]);
	lns[2] = Line(pnts[2], pnts[3]-pnts[2]);
	lns[3] = Line(pnts[3], pnts[0]-pnts[3]);
	lns[4] = Line(pnts[0], pnts[4]-pnts[0]);
	lns[5] = Line(pnts[7], pnts[3]-pnts[7]);
	lns[6] = Line(pnts[5], pnts[1]-pnts[5]);
	lns[7] = Line(pnts[2], pnts[6]-pnts[2]);
	lns[8] = Line(pnts[5], pnts[4]-pnts[5]);
	lns[9] = Line(pnts[6], pnts[5]-pnts[6]);
	lns[10] = Line(pnts[7],pnts[6]-pnts[7]);
	lns[11] = Line(pnts[4], pnts[7]-pnts[4]);

	return lns;
}

QVector<QVector<Point>> BBox::getBoxFaces()
{
	QVector<QVector<Point>> faces;
	QVector<Point> pnts = getBoxCorners();

	for (int i = 0; i < 6; i++)	{
		QVector<Vector3> conners;
		for (int j = 0; j < 4; j++)	{
			conners.push_back( pnts[cubeIds[i][j] ] );
		}
		faces.push_back(conners);
	}	

	return faces;
}

int BBox::getOrthoAxis(QVector<Point> &plane)
{
	for(int i = 0; i < 3; i++){
		Vector3 d01 = (plane[0] - plane[1]).normalized();
		Vector3 d03 = (plane[0] - plane[3]).normalized();
		if(dot(d01, Axis[i]) == 0 && dot(d03, Axis[i]) == 0){
			axisID = i;
			return i;
		}
	}
	return -1;
}

bool BBox::IntersectRayBox(Point &start, Vec3d &startDir, Point &intPnt)
{
	float tmin = 0.0f; // set to -FLT_MAX to get first hit on line
	Point p = getCoordinates(start);
	Vec3d d = getCoordinates(start+startDir) - p;
	computeBBMinMax();
	float tmax = FLT_MAX; // set to max distance ray can travel 
	// For all three slabs
	for (int i = 0; i < 3; i++) {
		if (fabs(d[i]) < FLT_EPSILON ) {
			// Ray is parallel to slab. No hit if origin not within slab
			if (p[i] < bbmin[i] || p[i] > bbmax[i]) return 0;
		} 
		else {
			// Compute intersection t value of ray with near and far plane of slab
			float ood = 1.0f / d[i];
			float t1 = (bbmin[i] - p[i]) * ood;
			float t2 = (bbmax[i] - p[i]) * ood;
			// Make t1 be intersection with near plane, t2 with far plane
			if (t1 > t2) {
				float tmp = t1;
				t1 = t2;
				t2 = tmp;
			}//Swap(t1, t2);
			// Compute the intersection of slab intersection intervals
			if (t1 > tmin) tmin = t1;
			if (t2 > tmax) tmax = t2;
			// Exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax) return false;
		}
	}
	// Ray intersects all 3 slabs. Return point (q) and intersection t value (tmin)
	intPnt = getPosition(p + d * tmin);
	if(isFaceContainPnt(intPnt)){
       isSelected = true;
	   return true;
	}
	return false;
}

void BBox::computeBBMinMax()
{
	bbmin = Point( FLT_MAX,  FLT_MAX,  FLT_MAX);
	bbmax = Point(-FLT_MAX, -FLT_MAX, -FLT_MAX);	

	QVector<Point> conners = getBoxCorners();
	QVector<Point> coords;
	foreach(Point p, conners) 
	{
		bbmin = minimize(bbmin, getCoordinates(p));
		bbmax = maximize(bbmax, getCoordinates(p));
	}		
}

bool BBox::isFaceContainPnt(Point &pnt)
{
	selPlane.clear();
	QVector<QVector<Point>> faces = getBoxFaces();
	Point coord = getCoordinates(pnt);
	foreach(QVector<Point> f, faces){
		double xmax = 0, xmin = 100000, ymax = 0, ymin = 100000, zmax = 0, zmin = 100000;
		foreach(Point p, f){
			Point pt = getCoordinates(p);
			if(pt[0] > xmax)
				xmax = pt[0];
			if(pt[0] < xmin)
				xmin = pt[0];
			if(pt[1] > ymax)
				ymax = pt[1];
			if(pt[1] < ymin)
				ymin = pt[1];
			if(pt[2] > zmax)
				zmax = pt[2];
			if(pt[2] < zmin)
				zmin = pt[2];
		}
		if(coord[0] <= xmax && coord[0] >= xmin &&
			coord[1] <= ymax && coord[1] >= ymin &&
			coord[2] <= zmax && coord[2] >= zmin){
            selPlane = f;
			isSelected = true;
			return true;
		}		
	}
    return false;
}

int BBox::manipulate(Point &start, Vec3d &startDir)
{
	Point intPnt;
	if(IntersectRayBox(start, startDir, intPnt)){
	   if(isFaceContainPnt(intPnt))
		  return getOrthoAxis(selPlane);
	}
	return -1;
}

void BBox::deform(double factor)
{
	if(axisID < 0 || axisID > 2)
		return;
	Extent[axisID] -= factor; 
	Center[axisID] -= factor/2;
}

void BBox::draw()
{
	QVector<QVector<Point>> faces = getBoxFaces();

	for(int i = 0; i < faces.size(); i++)
		DrawSquare(faces[i], false, 2, Vec4d(0,0.5,1,0.5));

	if(isSelected) 
		DrawSquare(selPlane, true, 3, Vec4d(1,1,0,0.8));
}

void BBox::DrawSquare(QVector<Point> &f, bool isOpaque, float lineWidth, const Vec4d &color)
{
	glEnable(GL_LIGHTING);

	if(isOpaque)
	{
		// Draw the filled square
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( 0.5f, 0.5f );

		glColor4f(color[0], color[1], color[2], RANGED(0, color[3], 1.0f));

		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBegin(GL_QUADS);

		Vec3d v10 = f[1] - f[0];
		Vec3d v20 = f[2] - f[0];
		Vec3d n = cross(v10 , v20).normalized(); 

		glNormal3d(n[0],n[1],n[2]);
		glVertex3d(f[0].x(),f[0].y(),f[0].z());
		glVertex3d(f[1].x(),f[1].y(),f[1].z());
		glVertex3d(f[2].x(),f[2].y(),f[2].z());
		glVertex3d(f[3].x(),f[3].y(),f[3].z());
		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	glDisable(GL_LIGHTING);

	// Draw the edges
	glLineWidth(lineWidth);

	glColor4f(color[0], color[1], color[2], color[3]);

	glBegin(GL_LINE_STRIP);
	glVertex3d(f[0].x(),f[0].y(),f[0].z());
	glVertex3d(f[1].x(),f[1].y(),f[1].z());
	glVertex3d(f[2].x(),f[2].y(),f[2].z());
	glVertex3d(f[3].x(),f[3].y(),f[3].z());
	glVertex3d(f[0].x(),f[0].y(),f[0].z());
	glEnd();

	glEnable(GL_LIGHTING);
}