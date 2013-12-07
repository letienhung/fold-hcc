#include "ScoffoldNode.h"

ScoffoldNode::ScoffoldNode( QString nid )
	: Node(nid)
{
	this->mesh = NULL;
}

void ScoffoldNode::setMesh( SurfaceMesh::SurfaceMeshModel *m )
{
	this->mesh = m;
}

void ScoffoldNode::setCtrlBox( Geom::Box &b )
{
	this->ctrlBox = b;
}
