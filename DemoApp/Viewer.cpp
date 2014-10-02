#include "Viewer.h"
#include "ui_Viewer.h"
#include <QDebug>

QTimer * timer = new QTimer;

Viewer::Viewer(QWidget *parent) : QGLViewer(parent), ui(new Ui::Viewer), fmanager(NULL), theta(0.0)
{
    ui->setupUi(this);

	//setGridIsDrawn();
	camera()->frame()->setSpinningSensitivity(100);

	camera()->setSceneRadius(2);
	camera()->showEntireScene();
	camera()->setUpVector(Vec(0,0,1));
	camera()->setPosition(Vec(3,-3,3));
	camera()->lookAt(Vec());

	connect(timer, SIGNAL(timeout()), SLOT(updateView()));
	timer->start(20);
}

Viewer::~Viewer()
{
    delete ui;
}

void Viewer::updateView()
{
	theta += 0.01;
	updateGL();
}

void Viewer::preDraw()
{
    QGLViewer::preDraw();

    bool isDrawFancyBackground = true;
    if( isDrawFancyBackground )
    {
        int w = width();
        int h = height();

        QColor c1 (208,212,240);
        QColor c2 (255,255,255);

        // Setup 2D
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, w, h, 0, 0.0, -1.0);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Draw gradient quad
        glDisable(GL_LIGHTING);
        glBegin(GL_QUADS);
        glColor3d(c1.redF(),c1.greenF(),c1.blueF()); glVertex2d(0,0); glVertex2d(w,0);
        glColor3d(c2.redF(),c2.greenF(),c2.blueF()); glVertex2d(w,h); glVertex2d(0,h);
        glEnd();

        // End 2D
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        // Background has no depth
        glClear(GL_DEPTH_BUFFER_BIT);
    }
}

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

void Viewer::draw()
{
	if( fmanager )
	{
		if( fmanager->property("isDone").toBool() )
		{
			DecScaff* dec = fmanager->shapeDec;

			if( !dec->keyframes.isEmpty() )
			{
				double t = abs( sin(theta) );
				int idx = t * (dec->keyframes.size()-1);

				fmanager->selectKeyframe( idx );
				fmanager->getSelKeyframe()->draw();
			}
		}
		else
		{
			fmanager->scaffold->draw();
		}

		// Draw folding direction
		{
			glEnable(GL_LIGHTING);

			this->aabb = fmanager->scaffold->computeAABB();

			double scale = aabb.radius() * 0.25;
			glPushMatrix();
			glTranslated(-scale * 0.75, -scale * 0.75, 0);

			Vec from = Vec( aabb.bbmin.data() );
			Vec to = from + (Vec( fmanager->sqzV.data() ) * scale);

			glColor3d(0,0,0);
			renderText(from.x,from.y,from.z, "Folding direction");
			glEnable( GL_MULTISAMPLE );
			
			glColor3d(0.8,0,0);
			drawArrow(to, from + Vec( fmanager->sqzV.data() ) * (scale * 0.4), 0.5);

			glPopMatrix();
		}
	}
}

void Viewer::setFoldManager(FoldManager * manager)
{
	if(!manager) return;

	double meshSize = manager->scaffold->computeAABB().radius();
	camera()->setSceneRadius( meshSize * 2 );
	camera()->showEntireScene();
	camera()->setUpVector(Vec(0,0,1));
	camera()->setPosition(Vec(-meshSize * 2, -meshSize * 2, meshSize * 1.25));
	camera()->lookAt(Vec());

	this->theta = 0.0;

	this->fmanager = manager;

	updateGL();
}
