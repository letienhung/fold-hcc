#include "UtilityGlobal.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>

QVector<Vector3> getMeshVertices( SurfaceMeshModel* mesh )
{
	QVector<Vec3d> pnts;	

	Surface_mesh::Vertex_property<Point> points = mesh->vertex_property<Point>("v:point");
	Surface_mesh::Vertex_iterator vit, vend = mesh->vertices_end();

	for (vit = mesh->vertices_begin(); vit != vend; ++vit)
		pnts.push_back(points[vit]);

	return pnts;
}

QString qStr( Vector3 v, char sep)
{
	return QString("%1%2%3%4%5").arg(v.x()).arg(sep).arg(v.y()).arg(sep).arg(v.z());
}

QString qStr( const Vector4 &v, char sep )
{
	return QString("%1%2%3%4%5%6%7").arg(v[0]).arg(sep).arg(v[1]).arg(sep).arg(v[2]).arg(sep).arg(v[3]);
}

Vector3 toVector3( QString string )
{
	QStringList sl = string.split(' ');
	if (sl.size() != 3) 
		return Vector3();
	else
		return Vector3(sl[0].toDouble(), sl[1].toDouble(), sl[2].toDouble());
}

void saveOBJ( SurfaceMesh::Model * mesh, QString filename )
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
	foreach( SurfaceMesh::Vertex v, mesh->vertices() )
		out << "v " << points[v][0] << " " << points[v][1] << " " << points[v][2] << "\n";
	foreach( SurfaceMesh::Face f, mesh->faces() ){
		out << "f ";
		Surface_mesh::Vertex_around_face_circulator fvit=mesh->vertices(f), fvend=fvit;
		do{	out << (((Surface_mesh::Vertex)fvit).idx()+1) << " ";} while (++fvit != fvend);
		out << "\n";
	}
	file.close();
}
