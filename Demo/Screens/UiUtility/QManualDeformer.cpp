#include "QManualDeformer.h"

#include "SimpleDraw.h"

QManualDeformer::QManualDeformer(BBox * b)
{
	this->frame = new qglviewer::ManipulatedFrame;
	this->mBox = b;
	this->frame->setSpinningSensitivity(100.0);

	this->connect(frame, SIGNAL(manipulated()), SLOT(updateBox()));
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

	Vec3d delta(0,0,0);

	Point p = pos();
	int i = mBox->getOrthoAxis(mBox->selPlane);
	Point center = (mBox->selPlane.Conners[0] + mBox->selPlane.Conners[1] + mBox->selPlane.Conners[2] + mBox->selPlane.Conners[3])/4;
    double factor = fabs(p[i] - center[i]);
	mBox->deform(i, factor);

	emit( objectModified() );
}

void QManualDeformer::draw()
{
	SimpleDraw::IdentifyPoint(pos(), 1,1,0,20);
}

