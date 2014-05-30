#pragma once

#include <qgl.h>
#include "SurfaceMeshModel.h"

#define glVector3( v ) glVertex3d( v.x(), v.y(), v.z() )
#define glNormal3( v ) glNormal3d( v.x(), v.y(), v.z() )

struct QuickMeshDraw{

	static void drawMeshSolid( SurfaceMeshModel * mesh, QColor c = Qt::gray )
	{
		if(!mesh) return;

		if(!mesh->property("hasNormals").toBool())
		{
			mesh->update_face_normals();
			mesh->update_vertex_normals();
			mesh->setProperty("hasNormals",true);
		}

		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LIGHTING);

		glColorQt(c);

		Surface_mesh::Vertex_property<Vector3> points = mesh->vertex_property<Vector3>("v:point");
		Surface_mesh::Face_property<Vector3> fnormals = mesh->face_property<Vector3>("f:normal");

		Surface_mesh::Face_iterator fit, fend = mesh->faces_end();
		Surface_mesh::Vertex_around_face_circulator fvit, fvend;

		glBegin(GL_TRIANGLES);
		for (fit=mesh->faces_begin(); fit!=fend; ++fit){
			glNormal3( fnormals[fit] );
			fvit = fvend = mesh->vertices(fit);
			do{ glVector3( points[fvit] ); } while (++fvit != fvend);
		}
		glEnd();
	}

	static void drawMeshWireFrame( SurfaceMeshModel * mesh )
	{
		if(!mesh) return;

		Surface_mesh::Face_iterator fit, fend = mesh->faces_end();
		Surface_mesh::Vertex_around_face_circulator fvit, fvend;
		Surface_mesh::Vertex_property<Vector3> points = mesh->vertex_property<Vector3>("v:point");
		Surface_mesh::Face_property<Vector3> fnormals = mesh->face_property<Vector3>("f:normal");

		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glColor4d(0,0,0,1);
		glLineWidth(1.0f);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		for (fit=mesh->faces_begin(); fit!=fend; ++fit){
			glBegin(GL_POLYGON);
			glNormal3( fnormals[fit] );
			fvit = fvend = mesh->vertices(fit);
			do{ glVector3( points[fvit] ); } while (++fvit != fvend);
			glEnd();
		}
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		glDisable(GL_CULL_FACE);
	}

	static void drawMeshSharpEdges( SurfaceMeshModel * mesh, QColor color, float lineWidth )
	{
		if(!mesh) return;

		Surface_mesh::Face_iterator fit, fend = mesh->faces_end();
		Surface_mesh::Vertex_around_face_circulator fvit, fvend;
		Surface_mesh::Vertex_property<Vector3> points = mesh->vertex_property<Vector3>("v:point");
		Surface_mesh::Face_property<Vector3> fnormals = mesh->face_property<Vector3>("f:normal");

		glDisable(GL_LIGHTING);

		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glColor4d(color.redF(),color.greenF(),color.blueF(),1);
		glLineWidth( lineWidth );
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		double threshold = cos( M_PI_4 );

		glBegin(GL_LINES);
		foreach(Edge e, mesh->edges())
		{
			Vector3 n0 = fnormals[mesh->face(mesh->halfedge(e,0))];
			Vector3 n1 = fnormals[mesh->face(mesh->halfedge(e,1))];

			if( dot( n0, n1 ) > threshold ) continue;

			Vector3 p0 = points[mesh->vertex(e, 0)];
			Vector3 p1 = points[mesh->vertex(e, 1)];

			glVertex3dv(p0.data());
			glVertex3dv(p1.data());
		}
		glEnd();

		glDisable(GL_CULL_FACE);

		glEnable(GL_LIGHTING);
	}

	static void drawMeshName( SurfaceMeshModel * mesh, int name = 0 )
	{
		glPushName( name );

		Surface_mesh::Vertex_property<Vector3> points = mesh->vertex_property<Vector3>("v:point");
		Surface_mesh::Face_iterator fit, fend = mesh->faces_end();
		Surface_mesh::Vertex_around_face_circulator fvit, fvend;

		glBegin(GL_TRIANGLES);
		for (fit=mesh->faces_begin(); fit!=fend; ++fit){
			fvit = fvend = mesh->vertices(fit);
			do{ glVector3( points[fvit] ); } while (++fvit != fvend);
		}
		glEnd();

		glPopName();
	}
};
