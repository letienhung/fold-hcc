#include "SimpleDraw.h"
#include "Macros.h"
#include <qgl.h>

// Bad includes.. needed for rotations for now
#include "QGLViewer/qglviewer.h"
#include "ColorMap.h"

void SimpleDraw::DrawBox(const Vec3d& center, float width, float length, float height, float r, float g, float b, float lineWidth)
{
	glDisable(GL_LIGHTING);

	glColor3f(r, g, b);

	Vec3d  c1, c2, c3, c4;
	Vec3d  bc1, bc2, bc3, bc4;

	c1 = Vec3d (width, length, height) + center;
	c2 = Vec3d (-width, length, height) + center;
	c3 = Vec3d (-width, -length, height) + center;
	c4 = Vec3d (width, -length, height) + center;

	bc1 = Vec3d (width, length, -height) + center;
	bc2 = Vec3d (-width, length, -height) + center;
	bc3 = Vec3d (-width, -length, -height) + center;
	bc4 = Vec3d (width, -length, -height) + center;

	glLineWidth(lineWidth);

	glBegin(GL_LINES);
	glVertex3d(c1[0],c1[1],c1[2]);glVertex3d(bc1[0],bc1[1],bc1[2]);
	glVertex3d(c2[0],c2[1],c2[2]);glVertex3d(bc2[0],bc2[1],bc2[2]);
	glVertex3d(c3[0],c3[1],c3[2]);glVertex3d(bc3[0],bc3[1],bc3[2]);
	glVertex3d(c4[0],c4[1],c4[2]);glVertex3d(bc4[0],bc4[1],bc4[2]);
	glVertex3d(c1[0],c1[1],c1[2]);glVertex3d(c2[0],c2[1],c2[2]);
	glVertex3d(c3[0],c3[1],c3[2]);glVertex3d(c4[0],c4[1],c4[2]);
	glVertex3d(c1[0],c1[1],c1[2]);glVertex3d(c4[0],c4[1],c4[2]);
	glVertex3d(c2[0],c2[1],c2[2]);glVertex3d(c3[0],c3[1],c3[2]);
	glVertex3d(bc1[0],bc1[1],bc1[2]);glVertex3d(bc2[0],bc2[1],bc2[2]);
	glVertex3d(bc3[0],bc3[1],bc3[2]);glVertex3d(bc4[0],bc4[1],bc4[2]);
	glVertex3d(bc1[0],bc1[1],bc1[2]);glVertex3d(bc4[0],bc4[1],bc4[2]);
	glVertex3d(bc2[0],bc2[1],bc2[2]);glVertex3d(bc3[0],bc3[1],bc3[2]);
	glEnd();

	glEnable(GL_LIGHTING);

}

void SimpleDraw::DrawSolidBox(const Vec3d & center, float width, float length, float height, float r, float g, float b, float a)
{
	glColor3f(r, g, b);

	Vec3d  c1, c2, c3, c4;
	Vec3d  bc1, bc2, bc3, bc4;

	width *= 0.5;
	length *= 0.5;
	height *= 0.5;

	c1 = Vec3d (width, length, height) + center;
	c2 = Vec3d (-width, length, height) + center;
	c3 = Vec3d (-width, -length, height) + center;
	c4 = Vec3d (width, -length, height) + center;

	bc1 = Vec3d (width, length, -height) + center;
	bc2 = Vec3d (-width, length, -height) + center;
	bc3 = Vec3d (-width, -length, -height) + center;
	bc4 = Vec3d (width, -length, -height) + center;

	glShadeModel(GL_FLAT);

	SimpleDraw::DrawSquare(c1, c2, c3, c4, true,1, r,g,b,a);
	SimpleDraw::DrawSquare(bc4, bc3, bc2, bc1, true,1, r,g,b,a);

	SimpleDraw::DrawSquare(c1, c4, bc4, bc1, true,1, r,g,b,a);
	SimpleDraw::DrawSquare(c2, c1, bc1, bc2, true,1, r,g,b,a);

	SimpleDraw::DrawSquare(c4, c3, bc3, bc4, true,1, r,g,b,a);
	SimpleDraw::DrawSquare(c2, bc2, bc3, c3, true,1, r,g,b,a);

	glShadeModel(GL_SMOOTH);
}

void SimpleDraw::DrawTriangle(const Vec3d & v1, const Vec3d & v2, const Vec3d & v3, float r, float g, float b, float a, bool isOpaque)
{
	glDisable(GL_LIGHTING);

	if(a < 1.0f)
	{
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Draw the filled triangle
	if(isOpaque)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( 0.5, 0.5 );

		glColor4f(r * 0.75f, g * 0.75f, b * 0.75f, RANGED(0,a,1.0f));

		if(a < 1.0f){
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glBegin(GL_TRIANGLES);
		glVertex3d(v1[0],v1[1],v1[2]);
		glVertex3d(v2[0],v2[1],v2[2]);
		glVertex3d(v3[0],v3[1],v3[2]);
		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	// Draw the edges
	glLineWidth(1.0f);

	glColor3f(r * 0.5f, g * 0.5f, b * 0.5f);

	glBegin(GL_LINE_STRIP);
	glVertex3d(v1[0],v1[1],v1[2]);
	glVertex3d(v2[0],v2[1],v2[2]);
	glVertex3d(v3[0],v3[1],v3[2]);
	glVertex3d(v1[0],v1[1],v1[2]);
	glEnd();

	// Draw the points
	int pointSize = 4;

	glEnable(GL_POINT_SMOOTH);

	// Colored dot
	glColor3f(r,g,b);
	glPointSize(pointSize);
	glBegin(GL_POINTS);
	glVertex3d(v1[0],v1[1],v1[2]);
	glVertex3d(v2[0],v2[1],v2[2]);
	glVertex3d(v3[0],v3[1],v3[2]);
	glEnd();

	// White Border
	glPointSize(pointSize + 2);
	glColor3f(1, 1, 1);

	glBegin(GL_POINTS);
	glVertex3d(v1[0],v1[1],v1[2]);
	glVertex3d(v2[0],v2[1],v2[2]);
	glVertex3d(v3[0],v3[1],v3[2]);
	glEnd();

	glDisable (GL_BLEND);

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawTriangles( const StdVector< StdVector<Vec3d> > & tris, float r, float g, float b, float a, bool isOpaque, bool isDrawVec3ds)
{
	glDisable(GL_LIGHTING);

	if(a < 1.0f)
	{
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Draw the filled triangle
	if(isOpaque)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( 0.5, 0.5 );

		glColor4f(r * 0.75f, g * 0.75f, b * 0.75f, RANGED(0,a,1.0f));

		if(a < 1.0f){
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glBegin(GL_TRIANGLES);
		for(int i = 0; i < (int)tris.size(); i++)
		{
			glVertex3d(tris[i][0].x(), tris[i][0].y(),tris[i][0].z());
			glVertex3d(tris[i][1].x(), tris[i][1].y(),tris[i][1].z());
			glVertex3d(tris[i][2].x(), tris[i][2].y(),tris[i][2].z());
		}
		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	// Draw the edges
	glLineWidth(1.0f);

	if(isOpaque)
		glColor3f(r * 0.5f, g * 0.5f, b * 0.5f);
	else
		glColor3f(r, g, b);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBegin(GL_TRIANGLES);
	for(int i = 0; i < (int)tris.size(); i++)
	{
		glVertex3d(tris[i][0].x(), tris[i][0].y(),tris[i][0].z());
		glVertex3d(tris[i][1].x(), tris[i][1].y(),tris[i][1].z());
		glVertex3d(tris[i][2].x(), tris[i][2].y(),tris[i][2].z());
	}
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if(isDrawVec3ds)
	{
		// Draw the points
		int pointSize = 4;

		glEnable(GL_POINT_SMOOTH);

		// Colored dot
		glColor3f(r,g,b);
		glPointSize(pointSize);
		glBegin(GL_POINTS);
		for(int i = 0; i < (int)tris.size(); i++)
		{
			glVertex3d(tris[i][0].x(), tris[i][0].y(),tris[i][0].z());
			glVertex3d(tris[i][1].x(), tris[i][1].y(),tris[i][1].z());
			glVertex3d(tris[i][2].x(), tris[i][2].y(),tris[i][2].z());
		}
		glEnd();

		// White Border
		glPointSize(pointSize + 2);
		glColor3f(1, 1, 1);

		glBegin(GL_POINTS);
		for(int i = 0; i < (int)tris.size(); i++)
		{
			glVertex3d(tris[i][0].x(), tris[i][0].y(),tris[i][0].z());
			glVertex3d(tris[i][1].x(), tris[i][1].y(),tris[i][1].z());
			glVertex3d(tris[i][2].x(), tris[i][2].y(),tris[i][2].z());
		}
		glEnd();
	}

	glDisable (GL_BLEND);

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawPoly( const std::vector<Vec3d> & poly, float r, float g, float b)
{	
	glDisable(GL_LIGHTING);

	glColor3f(r, g, b);

	float lineWidth = 15.0f;
	glLineWidth(lineWidth);

	glBegin(GL_LINE_STRIP);
	for(uint i = 0; i <= poly.size(); i++)
		glVertex3d(poly[i%poly.size()].x(),poly[i%poly.size()].y(),poly[i%poly.size()].z());
	glEnd();

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawLineTick(const StdVector<Vec3d >& start, const StdVector<Vec3d >& direction, 
							  float len, bool border, float r, float g, float b, float a)
{
	glPushAttrib( GL_ALL_ATTRIB_BITS );

	glDisable(GL_LIGHTING);

	if(a < 1.0f){
		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	if(border)
	{
		glClearStencil(0);
		glClear( GL_STENCIL_BUFFER_BIT );
		glEnable( GL_STENCIL_TEST );

		glStencilFunc( GL_ALWAYS, 1, 0xFFFF );
		glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glColor3f( 0.0f, 0.0f, 0.0f );
	}

	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glColor4f(r, g, b, a);
	for(int i = 0; i < (int)start.size(); i++)
	{
		glVertex3d(start[i].x(),start[i].y(),start[i].z());
		Vec3d end = start[i] + (direction[i] * len);
		glVertex3d(end.x(),end.y(),end.z());
	}
	glEnd();

	if(border)
	{
		glStencilFunc( GL_NOTEQUAL, 1, 0xFFFF );
		glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

		glLineWidth(4.0f);
		glBegin(GL_LINES);
		glColor4f(0, 0, 0, 1);
		for(int i = 0; i < (int)start.size(); i++)
		{
			glVertex3d(start[i].x(),start[i].y(),start[i].z());
			Vec3d end = start[i] + (direction[i] * len * 1.05f);
			glVertex3d(end.x(),end.y(),end.z());
		}
		glEnd();
	}


	glPopAttrib();

}

void SimpleDraw::DrawSquare(const std::vector<Vec3d> & v, bool isOpaque, float lineWidth, Vec4d &color)
{
	DrawSquare(v[0],v[1],v[2],v[3], isOpaque, lineWidth, color[0], color[1], color[2], color[3]);
}

void SimpleDraw::DrawSquare(const Vec3d & v1, const Vec3d & v2, const Vec3d & v3, const Vec3d & v4, 
	bool isOpaque, float lineWidth, Vec4d &color)
{
	DrawSquare(v1,v2,v3,v4, isOpaque, lineWidth, color[0], color[1], color[2], color[3]);
}

void SimpleDraw::DrawSquare(const Vec3d & v1, const Vec3d & v2, const Vec3d & v3, const Vec3d & v4, 
							bool isOpaque, float lineWidth, float r, float g, float b, float a)
{
	glEnable(GL_LIGHTING);

	if(isOpaque)
	{
		// Draw the filled square
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( 0.5f, 0.5f );

		glColor4f(r, g, b, RANGED(0, a, 1.0f));

		glEnable(GL_BLEND); 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBegin(GL_QUADS);
		
		Vec3d v21 = v2 - v1;
		Vec3d v31 = v3 - v1;
		Vec3d n = cross(v21 , v31).normalized();
		glNormal3d(n[0],n[1],n[2]);
		glVertex3d(v1[0],v1[1],v1[2]);
		glVertex3d(v2[0],v2[1],v2[2]);
		glVertex3d(v3[0],v3[1],v3[2]);
		glVertex3d(v4[0],v4[1],v4[2]);
		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	glDisable(GL_LIGHTING);

	// Draw the edges
	glLineWidth(lineWidth);

	glColor4f(r, g, b, a);

	glBegin(GL_LINE_STRIP);
	glVertex3d(v1[0],v1[1],v1[2]);
	glVertex3d(v2[0],v2[1],v2[2]);
	glVertex3d(v3[0],v3[1],v3[2]);
	glVertex3d(v4[0],v4[1],v4[2]);
	glVertex3d(v1[0],v1[1],v1[2]);
	glEnd();

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawSquares( const StdVector<StdVector<Vec3d > >& squares, bool isOpaque, float r, float g, float b, float a )
{
	glDisable(GL_LIGHTING);

	if(isOpaque)
	{
		// Draw the filled square
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset( 0.5, 0.5 );

		glColor4f(r * 0.75f, g * 0.75f, b * 0.75f, RANGED(0, a, 1.0f));

		if(a < 1.0f){
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glBegin(GL_QUADS);
		for(int i = 0; i < (int)squares.size(); i++)
		{
			glVertex3d(squares[i][0].x(),squares[i][0].y(),squares[i][0].z());
			glVertex3d(squares[i][1].x(),squares[i][1].y(),squares[i][1].z());
			glVertex3d(squares[i][2].x(),squares[i][2].y(),squares[i][2].z());
			glVertex3d(squares[i][3].x(),squares[i][3].y(),squares[i][3].z());
		}
		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	// Draw the edges
	glLineWidth(1.0f);

	glColor3f(r, g, b);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBegin(GL_QUADS);
	for(int i = 0; i < (int)squares.size(); i++)
	{
		glVertex3d(squares[i][0].x(),squares[i][0].y(),squares[i][0].z());
		glVertex3d(squares[i][1].x(),squares[i][1].y(),squares[i][1].z());
		glVertex3d(squares[i][2].x(),squares[i][2].y(),squares[i][2].z());
		glVertex3d(squares[i][3].x(),squares[i][3].y(),squares[i][3].z());
	}
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawCube( const Vec3d & center, float length /*= 1.0f*/ )
{
	static GLdouble n[6][3] ={
		{-1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{1.0, 0.0, 0.0},
		{0.0, -1.0, 0.0},
		{0.0, 0.0, 1.0},
		{0.0, 0.0, -1.0}};

	static GLint faces[6][4] ={
		{0, 1, 2, 3},
		{3, 2, 6, 7},
		{7, 6, 5, 4},
		{4, 5, 1, 0},
		{5, 6, 2, 1},
		{7, 4, 0, 3}};

	GLdouble v[8][3];GLint i;

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = -length / 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = length / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -length / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = length / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -length / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = length / 2;

	glPushMatrix();
	glTranslatef(center.x(), center.y(), center.z());

	for (i = 0; i < 6; i++) 
	{
		glBegin(GL_QUADS);
		glNormal3dv(&n[i][0]);
		glVertex3dv(&v[faces[i][0]][0]);
		glVertex3dv(&v[faces[i][1]][0]);
		glVertex3dv(&v[faces[i][2]][0]);
		glVertex3dv(&v[faces[i][3]][0]);
		glEnd();
	}

	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void SimpleDraw::DrawSphere( const Vec3d & center, float radius /*= 1.0f*/ )
{
	glPushMatrix();
	glTranslatef(center.x(), center.y(), center.z());
	GLUquadricObj *quadObj = gluNewQuadric();

	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricNormals(quadObj, GLU_SMOOTH);

	gluSphere(quadObj, radius, 16, 16);

	gluDeleteQuadric(quadObj);
	glPopMatrix();
}

void SimpleDraw::DrawSpheres( StdVector<Vec3d> & centers, float radius /*= 1.0*/ )
{
	GLUquadricObj *quadObj = gluNewQuadric();

	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricNormals(quadObj, GLU_SMOOTH);

	for(int i = 0; i < (int)centers.size(); i++)
	{
		glPushMatrix();
		glTranslatef(centers[i].x(), centers[i].y(), centers[i].z());
		gluSphere(quadObj, radius, 16, 16);
		glPopMatrix();
	}

	gluDeleteQuadric(quadObj);
}

void SimpleDraw::DrawCylinder( const Vec3d & center, const Vec3d & direction /*= Vec3d (0,0,1)*/,
							  float height, float radius /*= 1.0f*/, float radius2 /*= -1*/ )
{
	glPushMatrix();
	glTranslatef(center.x(), center.y(), center.z());
	glMultMatrixd(qglviewer::Quaternion(qglviewer::Vec(0,0,1), qglviewer::Vec(direction)).matrix());

	GLUquadricObj *quadObj = gluNewQuadric();
	gluQuadricDrawStyle(quadObj, GLU_FILL);
	gluQuadricNormals(quadObj, GLU_SMOOTH);

	if(radius2 < 0)	radius2 = radius;

	gluCylinder(quadObj, radius, radius2, height, 16, 16);

	gluDeleteQuadric(quadObj);
	glPopMatrix();
}

void SimpleDraw::DrawArrow(Vec3d  &from,Vec3d  &to, bool isForward /*= true*/ , bool isFilledBase, float width /*= 1.0f */)
{
	if(!isForward){
		Vec3d  temp = from;
		from = to;
		to = temp;
	}

	if(isFilledBase) 
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	float length = (from-to).norm();
	float radius = length * 0.05f;
	if (radius < 0.0) radius = 0.05 * length;

	// draw cube base
	DrawCube(from, radius * 3);

	glPushMatrix();
	glTranslatef(from[0],from[1],from[2]);
	glMultMatrixd(qglviewer::Quaternion(qglviewer::Vec(0,0,1), qglviewer::Vec(to-from)).matrix());

	static GLUquadric* quadric = gluNewQuadric();

	const float head = 2.5*(radius / length) + 0.1;
	const float coneRadiusCoef = 4.0 - 5.0 * head;

	gluCylinder(quadric, radius, radius, length * (1.0 - head/coneRadiusCoef), 16, 1);
	glTranslatef(0.0, 0.0, length * (1.0 - head));
	gluCylinder(quadric, coneRadiusCoef * radius, 0.0, head * length, 16, 1);

	glPopMatrix();
}

void SimpleDraw::DrawArrowDirected( Vec3d & pos, Vec3d & normal, float height /*= 1.0f*/, 
								   bool isForward /*= true*/ , bool isFilledBase, float width /*= 1.0f */)
{
	Vec3d v = pos + (normal*height);
	DrawArrow(pos, v, isForward, isFilledBase);
}

void SimpleDraw::DrawArrowDoubleDirected( Vec3d & pos, Vec3d & normal, float height /*= 1.0f*/, 
										 bool isForward /*= true*/, bool isFilledBase /*= true*/ )
{
	DrawArrowDirected(pos, normal, height, isForward, isFilledBase);

	glColor3f(1,0,0);
	Vec3d v = -normal;
	DrawArrowDirected(pos, v, height, isForward, isFilledBase);
}

void SimpleDraw::PointArrowAt( Vec3d  &point, float radius /*= 1.0f*/ )
{
	Vec3d v = point + (double(radius) * Vec3d (Max(0.2, point.x()),point.y(),point.z()).normalized());
	DrawArrowDirected(point, v, radius, false);
}

/*void SimpleDraw::IdentifyLines(StdVector<Line> & lines, float lineWidth, float r, float g, float b)
{
	glDisable(GL_LIGHTING);
	glLineWidth(lineWidth);

	glColor3f(r, g, b);

	glBegin(GL_LINES);
	for(int i = 0; i < (int)lines.size(); i++)
	{
		glVertex3dv(lines[i].a);
		glVertex3dv(lines[i].b);
	}
	glEnd();

	glEnable(GL_LIGHTING);
}*/
void SimpleDraw::IdentifyLine( const Vec3d & p1, const Vec3d & p2, Vec4d &c, bool showVec3ds /*= true*/, float lineWidth /*= 3.0f*/ )
{
	glDisable(GL_LIGHTING);

	// Set color
	glColor4d(c[0],c[1],c[2],c[3]);

	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	glVertex3d(p1[0], p1[1], p1[2]);
	glVertex3d(p2[0], p2[1], p2[2]);
	glEnd();

	if(showVec3ds)
	{
		// Draw colored end points
		glPointSize(lineWidth * 5);
		glBegin(GL_POINTS);
		glVertex3d(p1[0],p1[1],p1[2]);
		glVertex3d(p2[0],p2[1],p2[2]);
		glEnd();

		// White border end points
		glPointSize((lineWidth * 5) + 2);
		glColor3f(1, 1, 1);

		glBegin(GL_POINTS);
		glVertex3d(p1[0],p1[1],p1[2]);
		glVertex3d(p2[0],p2[1],p2[2]);
		glEnd();
	}

	glEnable(GL_LIGHTING);
}

void SimpleDraw::IdentifyDashedLine( const Vec3d & p1, const Vec3d & p2, Vec4d &c /*= Vec4d(1,0,0,1)*/, bool showVec3ds /*= true*/, float lineWidth /*= 3.0f */ )
{
	glLineStipple (1, 0xf0f0);  // Repeat count, repeat pattern
	glEnable (GL_LINE_STIPPLE); // Turn stipple on

	IdentifyLine(p1,p2,c,showVec3ds,lineWidth);

	glDisable (GL_LINE_STIPPLE); // Turn it back off
}

void SimpleDraw::IdentifyLines( const StdVector<Vec3d> & p1, const StdVector<Vec3d> & p2, Vec4d &c /*= Vec4d(1,0,0,1)*/, bool showVec3ds /*= true*/, float lineWidth /*= 3.0f */ )
{
	glDisable(GL_LIGHTING);

	// Set color
	glColor4d(c[0], c[1], c[2], c[3]);

	glLineWidth(lineWidth);
	glBegin(GL_LINES);
	for(int i = 0; i < (int)p1.size(); i++){
		glVertex3d(p1[i].x(),p1[i].y(),p1[i].z());
		glVertex3d(p2[i].x(),p2[i].y(),p2[i].z());
	}
	glEnd();

	if(showVec3ds)
	{
		// Draw colored end points
		glPointSize(lineWidth * 5);
		glBegin(GL_POINTS);
		for(int i = 0; i < (int)p1.size(); i++){
			glVertex3d(p1[i].x(),p1[i].y(),p1[i].z());
			glVertex3d(p2[i].x(),p2[i].y(),p2[i].z());
		}
		glEnd();

		// White border end points
		glPointSize((lineWidth * 5) + 2);
		glColor3f(1, 1, 1);

		glBegin(GL_POINTS);
		for(int i = 0; i < (int)p1.size(); i++){
			glVertex3d(p1[i].x(),p1[i].y(),p1[i].z());
			glVertex3d(p2[i].x(),p2[i].y(),p2[i].z());
		}
		glEnd();
	}

	glEnable(GL_LIGHTING);
}


void SimpleDraw::IdentifyPoint( const Vec3d & p, float r /*= 1.0*/, float g /*= 0.2f*/, float b /*= 0.2f*/, float pointSize /*= 10.0*/ )
{
	glDisable(GL_LIGHTING);

	// Colored dot
	glColor3f(r, g, b);
	glPointSize(pointSize);
	glBegin(GL_POINTS);
	glVertex3f(p.x(), p.y(), p.z());
	glEnd();

	// White Border
	glPointSize(pointSize + 2);
	glColor3f(1, 1, 1);

	glBegin(GL_POINTS);
	glVertex3f(p.x(), p.y(), p.z());
	glEnd();

	glEnable(GL_LIGHTING);
}

void SimpleDraw::IdentifyPoint2( Vec3d  &p )
{
	// Green
	IdentifyPoint(p, 0.2f, 1.0f, 0.2f, 12.0f);
}

void SimpleDraw::IdentifyPoints(const StdVector<Vec3d > & points, Vec4d &c, float pointSize)
{
	glDisable(GL_LIGHTING);

	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Colored dot
	glColor4d(c[0], c[1], c[2], c[3]);
	glPointSize(pointSize);
	glBegin(GL_POINTS);
	for(unsigned int i = 0; i < points.size(); i++)
		glVertex3d(points[i].x(),points[i].y(),points[i].z());
	glEnd();

	// White Border
	glPointSize(pointSize + 2);
	glColor4d(1, 1, 1, c[3]);

	glBegin(GL_POINTS);
	for(unsigned int i = 0; i < points.size(); i++)
		glVertex3d(points[i].x(),points[i].y(),points[i].z());
	glEnd();

	glEnable(GL_LIGHTING);
}

void SimpleDraw::IdentifyCurve( StdVector<Vec3d> & points, float r, float g, float b, float a, float lineWidth )
{
	glDisable(GL_LIGHTING);
	glColor4f(r, g, b, a);

	glLineWidth(lineWidth);

	glBegin(GL_LINE_STRIP);
	foreach(Vec3d p, points)
		glVertex3d(p[0],p[1],p[2]);
	glEnd();
}

void SimpleDraw::IdentifyConnectedPoints( StdVector<Vec3d> & points, float r /*= 0.4f*/, float g /*= 1.0*/, float b /*= 0.2f*/ )
{
	glDisable(GL_LIGHTING);
	glColor4f(r, g, b, 1);

	glLineWidth(2.0 + (r * 2) + (g * 4) + (b * 6));

	glBegin(GL_LINE_STRIP);
		foreach(Vec3d p, points)
			glVertex3d(p[0],p[1],p[2]);
	glEnd();

}

void SimpleDraw::IdentifyConnectedPoints2( StdVector<Vec3d > & points, float r /*= 0.4f*/, float g /*= 1.0*/, float b /*= 0.2f*/ )
{
	glDisable(GL_LIGHTING);

	int N = points.size();

	glLineWidth(3.0);
	glBegin(GL_LINE_STRIP);
	for(int i = 0; i < N; i++)
	{
		float t = Min((float(i) / N + 0.25f), 1.0);
		glColor3f(r * t, g * t, b * t);
		glVertex3d(points[i].x(),points[i].y(),points[i].z());
	}
	glEnd();

	// Colored dot
	glColor3f(r, g, b);
	glPointSize(13.0);
	glBegin(GL_POINTS);
	for(int i = 0; i < N; i++)
	{
		float t = float(i) / N;
		glColor3f(r * t, g * t, b * t);
		glVertex3d(points[i].x(),points[i].y(),points[i].z());
	}
	glEnd();

	// White Border
	glPointSize(15.0);
	glColor3f(1, 1, 1);

	glBegin(GL_POINTS);
	for(int i = 0; i < N; i++)
		glVertex3d(points[i].x(),points[i].y(),points[i].z());
	glEnd();

	glEnable(GL_LIGHTING);
}

void SimpleDraw::IdentifyLineRed( const Vec3d & p1, const Vec3d & p2, bool showVec3ds /*= true*/ )
{
	// Red line
	IdentifyLine(p1, p2, Vec4d(1.0, 0.2, 0.2, 1), showVec3ds);
}

void SimpleDraw::IdentifyArrow( const Vec3d  & start, const Vec3d  & end, float lineWidth /*= 2.0*/, float r /*= 1.0*/, float g /*= 0.2f*/, float b /*= 0.2f*/ )
{
	glDisable(GL_LIGHTING);

	// Transparency
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(lineWidth);

	glBegin(GL_LINES);

	glColor4f(r/2, g/2, b/2, 0.2f);
	glVertex3d(start[0],start[1],start[2]);

	glColor4f(r, g, b, 1.0);
	glVertex3d(end[0],end[1],end[2]);

	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glLineWidth(1.0);
}

void SimpleDraw::IdentifyArrows( StdVector<Vec3d > & starts, StdVector<Vec3d > & ends, float lineWidth /*= 2.0*/, float r /*= 1.0*/, float g /*= 0.2f*/, float b /*= 0.2f*/ )
{
	glDisable(GL_LIGHTING);

	// Transparency
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(lineWidth);

	glBegin(GL_LINES);
	for(unsigned int i = 0; i < starts.size(); i++)
	{
		glColor4f(r, g, b, 0.0);
		glVertex3d(starts[i].x(),starts[i].y(),starts[i].z());

		glColor4f(r, g, b, 1.0);
		glVertex3d(ends[i].x(),ends[i].y(),ends[i].z());
	}
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawBarChart(const StdVector<double> & data, int x, int y, double height, double width, int barWidth)
{
	glDisable(GL_LIGHTING);
	glLineWidth(barWidth);

	glPushMatrix();
	glTranslated(x,y,0);

	double scalingY = height / (1 + *max_element(data.begin(), data.end()));
	double scalingX = width * (1.0 / (int)data.size());

	glBegin(GL_LINES);
	for(int i = 0; i < (int)data.size(); i++)
	{
		glVertex2i(i * barWidth * scalingX, 0);
		glVertex2i(i * barWidth * scalingX, -data[i] * scalingY);
	}
	glEnd();

	glPopMatrix();

	glEnable(GL_LIGHTING);
}

void SimpleDraw::drawCornerAxis(const double * cameraOrientation)
{
	int viewport[4];
	int scissor[4];

	// The viewport and the scissor are changed to fit the lower left
	// corner. Original values are saved.
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetIntegerv(GL_SCISSOR_BOX, scissor);

	// Axis viewport size, in pixels
	const int size = 50;
	glViewport(0,0,size,size);
	glScissor(0,0,size,size);

	// The Z-buffer is cleared to make the axis appear over the
	// original image.
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMultMatrixd(cameraOrientation);

	// Tune for best line rendering
	glDisable(GL_LIGHTING);
	glLineWidth(3.0);

	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 1.0);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_LIGHTING);

	// The viewport and the scissor are restored.
	glScissor(scissor[0],scissor[1],scissor[2],scissor[3]);
	glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
}

QVector<Vec4d> SimpleDraw::RandomColors( int count )
{
	QVector<Vec4d> colors;
	for (int i = 0; i < count; i++){
		float r = ((rand() % 225) + 30) / 255.0f;
		float g = ((rand() % 230) + 25) / 255.0f;
		float b = ((rand() % 235) + 20) / 255.0f;
		Vec4d c(r, g, b, 1.0);
		colors.push_back(c);
	}
	return colors;
}

void SimpleDraw::DrawCircle( const Vec3d& center, double radius, const Vec4d& c, const Vec3d& n, float lineWidth )
{
	Vec3d startV(0,0,0);

	// Find orthogonal start vector
	if ((abs(n.y()) >= 0.9f * abs(n.x())) && 
		abs(n.z()) >= 0.9f * abs(n.x())) startV = Vec3d(0.0f, -n.z(), n.y());
	else if ( abs(n.x()) >= 0.9f * abs(n.y()) && 
		abs(n.z()) >= 0.9f * abs(n.y()) ) startV = Vec3d(-n.z(), 0.0f, n.x());
	else startV = Vec3d(-n.y(), n.x(), 0.0f);

	int segCount = 20;
	double theta = 2.0 * M_PI / segCount;

	glDisable(GL_LIGHTING);
	glLineWidth(lineWidth);
	glColor4d(c[0],c[1],c[2],c[3]);

	glBegin(GL_LINE_LOOP);
	for(int i = 0; i < segCount; i++){
		Vec3d vi = center + startV * radius;
		glVertex3d(vi[0],vi[1],vi[2]);
		ROTATE_VEC(startV, theta, n);
	}
	glEnd();

	glEnable(GL_LIGHTING);
}

void SimpleDraw::DrawCircles( const StdVector<Vec3d>& centers, const StdVector<double>& radius, const Vec4d& c, const Vec3d& normal, float lineWidth )
{
	for(int i = 0; i < (int) centers.size(); i++)
		DrawCircle(centers[i], radius[i], c, normal, lineWidth);
}