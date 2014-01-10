#include "FdNode.h"
#include "CustomDrawObjects.h"
#include "Numeric.h"
#include "AABB.h"
#include "MinOBB.h"
#include "QuickMeshDraw.h"
#include "MeshHelper.h"
#include "MeshBoolean.h"
#include "RodNode.h"
#include "PatchNode.h"
#include "PcaOBB.h"

FdNode::FdNode(QString id, Geom::Box &b, MeshPtr m )
	:Node(id)
{
	mMesh = m;
	origBox = b;
	mBox = b;
	encodeMesh();

	mColor = qRandomColor(); 
	mColor.setAlphaF(0.5);
	mType = NONE;

	showCuboids = true;
	showScaffold = true;
	showMesh = true;
}

FdNode::FdNode(FdNode& other)
	:Node(other)
{
	mMesh = other.mMesh;
	origBox = other.origBox;
	mBox = other.mBox;
	meshCoords = other.meshCoords;

	mColor = other.mColor;
	mType = other.mType;

	showCuboids = true;
	showScaffold = true; 
	showMesh = false;
}


FdNode::~FdNode()
{

}

void FdNode::draw()
{
	if (showScaffold)
	{
		drawScaffold();
	}

	if (showMesh)
	{
		drawMesh();
	}

	if (showCuboids)
	{
		if (properties.contains("virtual"))
			mBox.draw(QColor(255, 255, 255, 30));

		// faces
		mBox.draw(mColor);

		// wireframes
		if(isSelected)	
			mBox.drawWireframe(4.0, Qt::yellow);
		else if (!properties.contains("isCtrlPanel"))
			mBox.drawWireframe();
	}
}


void FdNode::drawMesh()
{
	if (mMesh.isNull()) return;

	deformMesh();
    QuickMeshDraw::drawMeshSolid(mMesh.data());//, QColor(102,178,255,255));//QColor(255,128,0,255));
	QuickMeshDraw::drawMeshWireFrame(mMesh.data());
}

void FdNode::encodeMesh()
{
	if (mMesh.isNull()) return;

	meshCoords = MeshHelper::encodeMeshInBox(mMesh.data(), origBox);
}

void FdNode::deformMesh()
{
	if (mMesh.isNull()) return;
	MeshHelper::decodeMeshInBox(mMesh.data(), mBox, meshCoords);
}

void FdNode::write( XmlWriter& xw )
{
	xw.writeOpenTag("node");
	{
		xw.writeTaggedString("type", QString::number(mType));
		xw.writeTaggedString("ID", this->mID);

		// box
		mBox.write(xw);
	}
	xw.writeCloseTag("node");
}

void FdNode::refit( BOX_FIT_METHOD method )
{
	if (mMesh.isNull()) return;

	QVector<Vector3> points = MeshHelper::getMeshVertices(mMesh.data());
	origBox = fitBox(points, method);

	mBox = origBox;
	encodeMesh();
	createScaffold();
}

Geom::AABB FdNode::computeAABB()
{
	return Geom::AABB(mBox.getConnerPoints());
}

void FdNode::drawWithName( int name )
{
	glPushName(name);
	mBox.draw();
	glPopName();
}

bool FdNode::isPerpTo( Vector3 v, double dotThreshold )
{
	Q_UNUSED(v);
	Q_UNUSED(dotThreshold);
	return false;
}

SurfaceMesh::Vector3 FdNode::center()
{
	return mBox.Center;
}

// clone partial node on the positive side of the chopper plane
FdNode* FdNode::cloneChopped( Geom::Plane chopper )
{
	// cut point along skeleton
	int aid = mBox.getClosestAxisId(chopper.Normal);
	Geom::Segment sklt = mBox.getSkeleton(aid);
	Vector3 cutPoint = chopper.getIntersection(sklt);
	Vector3 endPoint = (chopper.signedDistanceTo(sklt.P0) > 0) ?
						sklt.P0 : sklt.P1;

	// chop box
	Geom::Box box = mBox;
	box.Center = (cutPoint + endPoint) * 0.5;
	box.Extent[aid] = (cutPoint - endPoint).norm() * 0.5;

	// chop mesh
	// Geom::Box chopBox = box;
	// chopBox.Extent *= 1.001;
	// SurfaceMeshModel* mesh = MeshBoolean::cork(mMesh.data(), chopBox, MeshBoolean::ISCT);
	
	// create new nodes
	FdNode *choppedNode;
	if (mType == FdNode::ROD)
		choppedNode = new RodNode(mMesh->name, box, mMesh);
	else
		choppedNode = new PatchNode(mMesh->name, box, mMesh);
	choppedNode->meshCoords = meshCoords;

	return choppedNode;
}

QString FdNode::getMeshName()
{
	if (mMesh.isNull()) return "NULL";
	else return mMesh->name;
}

QVector<FdNode*> FdNode::getPlainNodes()
{
	return QVector<FdNode*>() << this;
}

void FdNode::cloneMesh()
{
	deformMesh();
	SurfaceMeshModel *mesh = new SurfaceMeshModel;
	
	foreach (Vector3 p, MeshHelper::getMeshVertices(mMesh.data())){
		mesh->add_vertex(p);
	}

	Surface_mesh::Face_iterator fit, fend = mMesh.data()->faces_end();
	for (fit = mMesh.data()->faces_begin(); fit != fend; fit++)
	{
		std::vector<Vertex> pnts; 
		Surface_mesh::Vertex_around_face_circulator vit = mMesh.data()->vertices(fit), vend = vit;
		do{ 
			int sub_vid = Vertex(vit).idx();
			pnts.push_back(Vertex(sub_vid)); 
		} while(++vit != vend);

		mesh->add_face(pnts);
	}

	mesh->update_face_normals();
	mesh->update_vertex_normals();
	mesh->updateBoundingBox();

	mMesh = MeshPtr(mesh);

	showMesh = true;
}
