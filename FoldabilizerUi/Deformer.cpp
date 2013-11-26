#include "Deformer.h"

Deformer::Deformer(Node *n)
{
	updateNode(n);
	mFactor = Vector3(0,0,0);
}

Deformer::~Deformer()
{
}

void Deformer::updateFactor(Vector3 &vec)
{
	mFactor = vec;
}

void Deformer::updateRotMat(Eigen::Matrix3f &m)
{
	mMat = m;
}

void Deformer::updateRotQuan(qglviewer::Quaternion &q)
{
	mQuan = q;
}

void Deformer::Deform(DEFORM_MODE mode)
{
	switch(mode){
	case SCALEMODE:
		scale(); break;
	case TRANSMODE:
		translate(); break;
	case ROTATEMODE:
		rotate(); break;
	}
}

void Deformer::updateNode(Node *n)
{
	mNode = n;
}

void Deformer::scale()
{
	mNode->scaleFactor = mFactor + Vector3(1.0,1.0,1.0);
	mNode->mBox.Extent = Vector3(mNode->mBox.Extent[0]*mNode->scaleFactor[0], 
		                         mNode->mBox.Extent[1]*mNode->scaleFactor[1],
								 mNode->mBox.Extent[2]*mNode->scaleFactor[2]);
}

void Deformer::translate()
{
	mNode->translate(Vector3(mFactor.x() * mNode->mBox.Extent.x(), 
		                     mFactor.y() * mNode->mBox.Extent.y(),
							 mFactor.z() * mNode->mBox.Extent.z()));
}

void Deformer::rotate()
{
	//mNode->rotate( mMat);
	mNode->rotate(mQuan);
}

