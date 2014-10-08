#include "ScaffNode.h"
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

ScaffNode::ScaffNode(QString id)
	: Node(id)
{
	mType = NONE;
	setRandomColor();

	showCuboid = true;
	showScaffold = true;
	showMesh = true;

	mAid = 0;

	isHidden = false;

}

ScaffNode::ScaffNode(QString id, Geom::Box &b, MeshPtr m )
	:Node(id)
{
	mMesh = m;
	origBox = b;
	mBox = b;
	encodeMesh();

	mType = NONE;
	setRandomColor();

	showCuboid = true;
	showScaffold = true;
	showMesh = true;

	mAid = 0;

	isHidden = false;
}

ScaffNode::ScaffNode(ScaffNode& other)
	:Node(other)
{
	mMesh = other.mMesh;
	origBox = other.origBox;
	mBox = other.mBox;
	meshCoords = other.meshCoords;

	mColor = other.mColor;
	mType = other.mType;

	showCuboid = true;
	showScaffold = true; 
	showMesh = false;

	mAid = other.mAid;
	isHidden = other.isHidden;
}


ScaffNode::~ScaffNode()
{

}


void ScaffNode::setRandomColor()
{
	mColor = qRandomColor3(0, 0.5, 0.7);
	mColor.setAlphaF(0.78);
}


void ScaffNode::draw()
{
	if (isHidden) return;

	if (showScaffold)
	{
		drawScaffold();
	}

	if (showMesh)
	{
		drawMesh();
	}

	if (showCuboid)
	{
		// faces
		mBox.draw(mColor);

		// wire frames
		if(isSelected)	
			mBox.drawWireframe(4.0, Qt::yellow);
		else
			mBox.drawWireframe(2.0, mColor.darker());
	}
}


void ScaffNode::drawMesh()
{
	if (mMesh.isNull()) return;

	deformMesh();
    QuickMeshDraw::drawMeshSolid(mMesh.data(), QColor(236,236,236));//, QColor(102,178,255,255));//QColor(255,128,0,255));
	//QuickMeshDraw::drawMeshWireFrame(mMesh.data());
	//QuickMeshDraw::drawMeshSharpEdges(mMesh.data(), QColor(0,0,0,255), 4);
}

void ScaffNode::encodeMesh()
{
	if (mMesh.isNull()) return;

	meshCoords = MeshHelper::encodeMeshInBox(mMesh.data(), origBox);
}

void ScaffNode::deformMesh()
{
	if (mMesh.isNull()) return;
	MeshHelper::decodeMeshInBox(mMesh.data(), mBox, meshCoords);
}

void ScaffNode::write( XmlWriter& xw )
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

void ScaffNode::refit( BOX_FIT_METHOD method )
{
	if (mMesh.isNull()) return;

	QVector<Vector3> points = MeshHelper::getMeshVertices(mMesh.data());
	origBox = fitBox(points, method);

	mBox = origBox;
	encodeMesh();
	createScaffold(false);
}

Geom::AABB ScaffNode::computeAABB()
{
	return Geom::AABB(mBox.getConnerPoints());
}

void ScaffNode::drawWithName( int name )
{
	glPushName(name);
	mBox.draw();
	glPopName();
}

bool ScaffNode::isPerpTo( Vector3 v, double dotThreshold )
{
	Q_UNUSED(v);
	Q_UNUSED(dotThreshold);
	return false;
}

SurfaceMesh::Vector3 ScaffNode::center()
{
	return mBox.Center;
}

// clone partial node on the positive side of the chopper plane
ScaffNode* ScaffNode::cloneChopped( Geom::Plane& chopper )
{
	// cut point along skeleton
	int aid = mBox.getAxisId(chopper.Normal);
	Geom::Segment sklt = mBox.getSkeleton(aid);
	Vector3 cutPoint = chopper.getIntersection(sklt);
	Vector3 endPoint = (chopper.signedDistanceTo(sklt.P0) > 0) ?
						sklt.P0 : sklt.P1;

	// chop box
	Geom::Box chopBox = mBox;
	chopBox.Center = (cutPoint + endPoint) * 0.5;
	chopBox.Extent[aid] = (cutPoint - endPoint).norm() * 0.5;

	return cloneChopped(chopBox);
}

ScaffNode* ScaffNode::cloneChopped( Geom::Plane& chopper1, Geom::Plane& chopper2 )
{
	// cut point along skeleton
	int aid = mBox.getAxisId(chopper1.Normal);
	Geom::Segment sklt = mBox.getSkeleton(aid);
	Vector3 cutPoint1 = chopper1.getIntersection(sklt);
	Vector3 cutPoint2 = chopper2.getIntersection(sklt);

	// chop box
	Geom::Box chopBox = mBox;
	chopBox.Center = (cutPoint1 + cutPoint2) * 0.5;
	chopBox.Extent[aid] = (cutPoint1 - cutPoint2).norm() * 0.5;

	return cloneChopped(chopBox);
}

ScaffNode* ScaffNode::cloneChopped( Geom::Box& chopBox )
{
	ScaffNode *choppedNode;
	if (mType == ScaffNode::ROD)
		choppedNode = new RodNode(mMesh->name, chopBox, mMesh);
	else
		choppedNode = new PatchNode(mMesh->name, chopBox, mMesh);
	choppedNode->meshCoords = meshCoords;

	return choppedNode;
}

QString ScaffNode::getMeshName()
{
	if (mMesh.isNull()) return "nullptr";
	else return mMesh->name;
}

QVector<ScaffNode*> ScaffNode::getSubNodes()
{
	return QVector<ScaffNode*>() << this;
}

void ScaffNode::cloneMesh()
{
	deformMesh();
	SurfaceMeshModel *mesh = new SurfaceMeshModel;
	
	for (Vector3 p : MeshHelper::getMeshVertices(mMesh.data())){
		mesh->add_vertex(p);
	}

	Surface_mesh::Face_iterator fit, fend = mMesh.data()->faces_end();
    for (fit = mMesh.data()->faces_begin(); fit != fend; ++fit)
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

void ScaffNode::exportMesh(QFile &file, int &v_offset)
{
	cloneMesh();
	QTextStream out(&file);
	out << "# Object " << this->mID << " #\n";
	out << "# NV = " << mMesh->n_vertices() << " NF = " << mMesh->n_faces() << "\n";
	SurfaceMesh::Vector3VertexProperty points = mMesh->vertex_property<Vector3>("v:point");
	for (SurfaceMesh::Vertex v : mMesh->vertices())
		out << "v " << points[v][0] << " " << points[v][1] << " " << points[v][2] << "\n";
	out << "g " << this->mID << "\n";
	for (SurfaceMesh::Face f : mMesh->faces()){
		out << "f ";
		Surface_mesh::Vertex_around_face_circulator fvit=mMesh->vertices(f), fvend=fvit;
		do{	out << (((Surface_mesh::Vertex)fvit).idx()+1+v_offset) << " ";} while (++fvit != fvend);
		out << "\n";
	}
	out << "\n";

	v_offset += mMesh->n_vertices();
}

void ScaffNode::deformToAttach( Geom::Plane& plane )
{
	int aid = mBox.getAxisId(plane.Normal);
	Geom::Segment sklt = mBox.getSkeleton(aid);

	double d0 = plane.signedDistanceTo(sklt.P0);
	double d1 = - plane.signedDistanceTo(sklt.P1);
	double a = d0 / (d0 + d1);
	double b = d1 / (d0 + d1);
	Vector3 p = a * sklt.P1 + b * sklt.P0;
	
	if (fabs(a) > fabs(b))
	{// p0 - p
		mBox.Center = 0.5 * (sklt.P0 + p);
		mBox.Extent[aid] = 0.5 * (sklt.P0 - p).norm();
	}
	else
	{// p1 - p
		mBox.Center = 0.5 * (sklt.P1 + p);
		mBox.Extent[aid] = 0.5 * (sklt.P1 - p).norm();
	}

	// deform
	createScaffold(true);
	deformMesh();
}

// mesh is not translated
// call deformMesh to update the location of mesh
void ScaffNode::translate( Vector3 t )
{
	mBox.translate(t);
	createScaffold(true);
}

void ScaffNode::setThickness( double thk )
{
    Q_UNUSED(thk)
	// do nothing
}

void ScaffNode::setShowCuboid( bool show )
{
	showCuboid = show;
}

void ScaffNode::setShowScaffold( bool show )
{
	showScaffold = show;
}

void ScaffNode::setShowMesh( bool show )
{
	showMesh = show;
}