#pragma once

#include "BBox.h"
#include "QGLViewer/qglviewer.h"

using namespace SurfaceMesh;

class QManualDeformer: public QObject{
	Q_OBJECT
public:
	QManualDeformer(BBox * b);

	qglviewer::ManipulatedFrame * getFrame();

	Vec3d pos();
	void draw();

public slots:
	void updateBox();

signals:
	void objectModified();

private:
	qglviewer::ManipulatedFrame *frame;
	BBox * mBox;
};
