#include "QManualDeformer.h"

#include "SimpleDraw.h"

QManualDeformer::QManualDeformer(BBox * b)
{
	this->frame = new qglviewer::ManipulatedFrame;
	this->mBox = b;
	this->frame->setSpinningSensitivity(100.0);

	//this->connect(frame, SIGNAL(manipulated()), SLOT(updateBox()));
}

qglviewer::ManipulatedFrame * QManualDeformer::getFrame()
{
	return frame;
}

Vec3d QManualDeformer::pos()
{
	qglviewer::Vec q = frame->position();
	return Vec3d (q.x,q.y,q.z);
}

void QManualDeformer::updateBox()
{
	if(!mBox) return;
	if(mBox->selPlaneID < 0)
		return;

	Vec3d delta(0,0,0);

	Point p = pos();
	Point center = mBox->getSelectedFace().Center;
    double factor = (p - center).norm()/mBox->Extent[mBox->axisID];
	mBox->deform(factor);

	emit( objectModified() );
}

void QManualDeformer::draw()
{
	SimpleDraw::IdentifyPoint(pos(), 1,1,0,20);
}

