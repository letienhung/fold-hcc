#include "SegMeshLoader.h"
#include <QFile>

SegMeshLoader::SegMeshLoader( SurfaceMesh::SurfaceMeshModel * mesh )
{
	this->entireMesh = mesh;
	entirePoints = entireMesh->vertex_property<Vector3>("v:point");
}

void SegMeshLoader::loadGroupsFromObj()
{
	// Read obj file
	QFile file(entireMesh->path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
	QTextStream inF(&file);

	int fidx = 0;
	while( !inF.atEnd() )
	{
		QString line = inF.readLine();
		if(line.isEmpty()) continue;

		if (line.startsWith("g"))
		{
			QStringList groupLine = line.split(" ");

			QString gid;
			if(!groupLine.isEmpty()) gid = groupLine.at(1);
			else gid = QString::number(groupFaces.size());

			while(true)
			{
				QString line = inF.readLine();
				if(!line.startsWith("f")) break;

				groupFaces[gid].push_back(fidx++);
			}
		}
	}
}


SurfaceMeshModel* SegMeshLoader::extractSegMesh( QString gid )
{
	SurfaceMeshModel* subMesh = new SurfaceMeshModel(gid + ".obj", gid);

	QVector<int> part = groupFaces[gid];
	
	// vertex set from face
	QSet<int> vertSet;
	foreach(int fidx, part)
	{
		Surface_mesh::Vertex_around_face_circulator vit = entireMesh->vertices(Face(fidx)),vend=vit;
		do{ vertSet.insert(Vertex(vit).idx()); } while(++vit != vend);
	}

	// entire vid => segment vid
	QMap<Vertex,Vertex> vmap;
	foreach(int vidx, vertSet)
	{
		vmap[Vertex(vidx)] = Vertex(vmap.size());
		subMesh->add_vertex( entirePoints[Vertex(vidx)] );
	}

	// copy vertices to subMesh
	foreach(int fidx, part){
		std::vector<Vertex> pnts; 
		Surface_mesh::Vertex_around_face_circulator vit = entireMesh->vertices(Face(fidx)),vend=vit;
		do{ pnts.push_back(vmap[vit]); } while(++vit != vend);
		subMesh->add_face(pnts);
	}

	subMesh->updateBoundingBox();
	subMesh->isVisible = true;

	return subMesh;
}

QVector<SurfaceMeshModel*> SegMeshLoader::getSegMeshes()
{
	loadGroupsFromObj();

	QVector<SurfaceMeshModel*> meshes;
	foreach (QString gid, groupFaces.keys())
		meshes.push_back(extractSegMesh(gid));

	return meshes;
}
