#ifndef VIEWER_H
#define VIEWER_H

#include "FoldManager.h"
#include <qglviewer/qglviewer.h>
using namespace qglviewer;

namespace Ui {
class Viewer;
}

class Viewer : public QGLViewer
{
    Q_OBJECT

public:
    explicit Viewer(QWidget *parent = 0);
    ~Viewer();

    void preDraw();
	void draw();

	FoldManager * fmanager;
	Geom::AABB aabb;
	double theta;

private:
    Ui::Viewer *ui;

public slots:
	void setFoldManager(FoldManager * manager);
	void updateView();
};

inline void DrawArrow( Vec3d  from, Vec3d  to, bool isForward /*= true*/ , bool isFilledBase, float /*width = 1.0f */)
{
	if(!isForward){
		Vec3d  temp = from;
		from = to;
		to = temp;
	}

	if(isFilledBase) 
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	float length = (from-to).norm();
	float radius = length * 0.05f;
	if (radius < 0.0) radius = 0.05 * length;

	// draw cube base
	//DrawCube(from, radius * 3);

	glPushMatrix();
	glTranslatef(from[0],from[1],from[2]);
	glMultMatrixd(qglviewer::Quaternion(qglviewer::Vec(0,0,1), qglviewer::Vec(to-from)).matrix());

	static GLUquadric* quadric = gluNewQuadric();

	const float head = 2.5*(radius / length) + 0.1;
	const float coneRadiusCoef = 4.0 - 5.0 * head;

	gluCylinder(quadric, radius, radius, length * (1.0 - head/coneRadiusCoef), 16, 1);
	glTranslatef(0.0, 0.0, length * (1.0 - head));
	gluCylinder(quadric, coneRadiusCoef * radius, 0.0, head * length, 16, 1);

	glPopMatrix();
}

#endif // VIEWER_H
