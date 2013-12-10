#include "AABB.h"
#include "Numeric.h"

Geom::AABB::AABB()
{
}

Geom::AABB::AABB( QVector<Vector3>& pnts )
{
	buildFromPoints(pnts);       
}

Geom::AABB::AABB( SurfaceMeshModel* mesh)
{
	buildFromMesh(mesh);
}

SurfaceMesh::Vector3 Geom::AABB::center()
{
	return (bbmin + bbmax) * 0.5f;
}

double Geom::AABB::radius()
{
	return (bbmax - bbmin).norm() * 0.5f;
}

Geom::Box Geom::AABB::box()
{
	Box b;
	b.Center = center();
	b.Axis = XYZ();
	b.Extent = (bbmax - bbmin) * 0.5f;

	return b;
}

void Geom::AABB::buildFromPoints( QVector<Vector3>& pnts )
{
	bbmin = Point( FLT_MAX,  FLT_MAX,  FLT_MAX);
	bbmax = Point(-FLT_MAX, -FLT_MAX, -FLT_MAX);    


	foreach(Point p, pnts) {
		bbmin = minimize(bbmin, p);
		bbmax = maximize(bbmax, p);
	} 
}

void Geom::AABB::buildFromMesh( SurfaceMeshModel* mesh )
{
	// Get points
	QVector<Vec3d> pnts;

	Surface_mesh::Vertex_property<Point> points = mesh->vertex_property<Point>("v:point");
	Surface_mesh::Vertex_iterator vit, vend = mesh->vertices_end();

	for (vit = mesh->vertices_begin(); vit != vend; ++vit)
		pnts.push_back(points[vit]);

	buildFromPoints(pnts);
}
