#include "MeshHelper.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include "Numeric.h"

QVector<Vector3> MeshHelper::getMeshVertices( SurfaceMeshModel* mesh )
{
	QVector<Vec3d> pnts;	

	Surface_mesh::Vertex_property<Point> points = mesh->vertex_property<Point>("v:point");
	Surface_mesh::Vertex_iterator vit, vend = mesh->vertices_end();

	for (vit = mesh->vertices_begin(); vit != vend; ++vit)
		pnts.push_back(points[vit]);

	return pnts;
}


void MeshHelper::saveOBJ( SurfaceMeshModel* mesh, QString filename )
{
	QFile file(filename);

	// Create folder
	QFileInfo fileInfo(file.fileName());
	QDir d(""); d.mkpath(fileInfo.absolutePath());

	// Open for writing
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

	QTextStream out(&file);
	out << "# NV = " << mesh->n_vertices() << " NF = " << mesh->n_faces() << "\n";
	SurfaceMesh::Vector3VertexProperty points = mesh->vertex_property<Vector3>("v:point");
	for (SurfaceMesh::Vertex v : mesh->vertices())
		out << "v " << points[v][0] << " " << points[v][1] << " " << points[v][2] << "\n";
	for (SurfaceMesh::Face f : mesh->faces()){
		out << "f ";
		Surface_mesh::Vertex_around_face_circulator fvit=mesh->vertices(f), fvend=fvit;
		do{	out << (((Surface_mesh::Vertex)fvit).idx()+1) << " ";} while (++fvit != fvend);
		out << "\n";
	}
	file.close();
}

QVector<Vector3> MeshHelper::encodeMeshInBox( SurfaceMeshModel* mesh, Geom::Box& box )
{
	QVector<Vector3> coords;

	for (Vector3 p : getMeshVertices(mesh))
		coords.push_back(box.getCoordinates(p));

	return coords;
}

bool MeshHelper::decodeMeshInBox( SurfaceMeshModel* mesh, Geom::Box& box, QVector<Vector3>& coords )
{
	if (mesh->n_vertices() != coords.size()) return false;
	if (coords.isEmpty()) return true;

	Surface_mesh::Vertex_property<Point> points = mesh->vertex_property<Point>("v:point");
	for(int i = 0; i < (int)mesh->n_vertices(); i++)
		points[Surface_mesh::Vertex(i)] = box.getPosition(coords[i]);

	return true;
}

void MeshHelper::deformMeshByBoxes( SurfaceMeshModel* mesh, Geom::Box& fromBox, Geom::Box& toBox )
{
	decodeMeshInBox(mesh, toBox, encodeMeshInBox(mesh, fromBox));
}