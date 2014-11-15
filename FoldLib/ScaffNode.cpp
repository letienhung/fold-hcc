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
#include "ParSingleton.h"

ScaffNode::ScaffNode(QString id)
	: Node(id)
{
	mType = NONE;
	setRandomColor();

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


void ScaffNode::setColor(QColor c)
{
	mColor = c;
	mColor.setAlphaF(0.78);
}


void ScaffNode::draw()
{
	if (isHidden) return;

	ParSingleton* fp = ParSingleton::instance();
	if (fp->showScaffold)
	{
		drawScaffold();
	}

	if (fp->showMesh)
	{
		drawMesh();
	}

	if (fp->showCuboid)
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

void ScaffNode::exportIntoXml( XmlWriter& xw )
{
	xw.writeOpenTag("node");
	{
		xw.writeTaggedString("type", QString::number(mType));
		xw.writeTaggedString("ID", this->mID);
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

ScaffNode* ScaffNode::cloneChoppedBetween(Vector3 p0, Vector3 p1)
{
	Vector3 p0p1 = p0 - p1;
	int aid = mBox.getAxisId(p0p1);
	Geom::Segment sklt = mBox.getSkeleton(aid);
	double t0 = sklt.getProjCoordinates(p0);
	double t1 = sklt.getProjCoordinates(p1);

	if ((t0 <= 0 && t1 <= 0) || (t0 >= 1 && t1 >= 1))
		return nullptr;
	else
	{
		t0 = RANGED(0, t0, 1);
		t1 = RANGED(0, t1, 1);

		Geom::Box chopBox = mBox;
		double tc = (t0 + t1) * 0.5;
		chopBox.Center = sklt.getPosition01(tc);
		double scale = fabs(t0 - t1);
		chopBox.Extent[aid] *= scale;

		ScaffNode* cloned = (ScaffNode*)clone();
		cloned->setBox(chopBox);
		return cloned;
	}
}


ScaffNode* ScaffNode::cloneChoppedAside(Vector3 p0, Vector3 d)
{
	int aid = mBox.getAxisId(d);
	Geom::Segment sklt = mBox.getSkeleton(aid);
	Vector3 p1 = (dot(d, sklt.Direction) > 0) ? sklt.P1 : sklt.P0;

	return cloneChoppedBetween(p0, p1);
}


QString ScaffNode::getMeshName()
{
	if (mMesh.isNull()) return "nullptr";
	else return mMesh->name;
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
}

void ScaffNode::exportIntoWholeMesh(QFile &wholeMeshFile, int &v_offset)
{
	cloneMesh();
	QTextStream out(&wholeMeshFile);
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
}

void ScaffNode::deformToAttach( PatchNode* pnode )
{
	int side = pnode->mPatch.whichSide(this->center());
	Geom::Plane surface = pnode->getSurfacePlane(side > 0);
	deformToAttach(surface);
}

void ScaffNode::translate( Vector3 t )
{
	mBox.translate(t);
	createScaffold(true);
}

void ScaffNode::setBoxFrame(Geom::Frame frame)
{
	mBox.setFrame(frame);
	createScaffold(true);
}

void ScaffNode::setBox(Geom::Box box)
{
	mBox = box;
	createScaffold(true);
}


void ScaffNode::scale01(int aid, double t0, double t1)
{
	Geom::Box chopBox = mBox;
	mBox.scale()

	double tc = (t0 + t1) * 0.5;
	chopBox.Center = sklt.getPosition01(tc);
	double scale = fabs(t0 - t1);
	chopBox.Extent[aid] *= scale;
}


void ScaffNode::exportMeshIndividually(QString meshesFolder)
{
	deformMesh();
	MeshHelper::saveOBJ(mMesh.data(), meshesFolder + '/' + mID + ".obj");
}
