#include "FdNode.h"

FdNode::FdNode( QString nid )
	: Node(nid)
{
	this->mesh = NULL;
}

void FdNode::setMesh( SurfaceMesh::SurfaceMeshModel *m )
{
	this->mesh = m;
}

void FdNode::setCtrlBox( Geom::Box &b )
{
	this->ctrlBox = b;
}
