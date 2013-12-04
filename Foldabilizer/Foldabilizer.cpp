#include "Foldabilizer.h"
#include "FoldabilizerWidget.h"
#include "StarlabDrawArea.h"
#include <QFileDialog>
#include <QDebug>

QString DEFAULT_FILE_PATH = "..\\..\\data";

#include "Numeric.h"

// test
#include "IntrRectRect.h"

Foldabilizer::Foldabilizer()
{
	widget = NULL;
	hccManager = new HccManager();
	this->connect(hccManager, SIGNAL(activeHccChanged()), SLOT(resetScene()));

	mhOptimizer = new MHOptimizer(hccManager);
	this->connect(mhOptimizer, SIGNAL(hccChanged()), SLOT(updateScene()));
}

void Foldabilizer::create()
{
	if (!widget)
	{
        widget = new FoldabilizerWidget(this);

		ModePluginDockWidget *dockwidget = new ModePluginDockWidget("Foldabilizer", mainWindow());
		dockwidget->setWidget(widget);
		mainWindow()->addDockWidget(Qt::RightDockWidgetArea, dockwidget);
	}
}

void Foldabilizer::destroy()
{

}

void Foldabilizer::decorate()
{
	if (hccManager)
		hccManager->draw();
}

void Foldabilizer::updateScene()
{
	drawArea()->updateGL();
}

void Foldabilizer::resetScene()
{
	if (hccManager)
		drawArea()->setSceneBoundingBox(qglviewer::Vec(hccManager->activeHcc()->bbmin), qglviewer::Vec(hccManager->activeHcc()->bbmax));
	
	if (mhOptimizer)
		mhOptimizer->isReady = false;

	drawArea()->camera()->showEntireScene();
	drawArea()->updateGL();
}

void Foldabilizer::loadGraph()
{
	QString fileName = QFileDialog::getOpenFileName(this->widget, "Import Mesh", DEFAULT_FILE_PATH, "Mesh Files (*.lcc)"); 
	if (fileName.isNull()) return;

	hccManager->loadHCC(fileName);

	DEFAULT_FILE_PATH = QFileInfo(fileName).absolutePath();
}

void Foldabilizer::test()
{
	qDebug() << "k\t\tn\t\tL0\t\tL1";

	for (int k = 3; k < 100; k++)
	{
		int n = 2*k - 1;
		double m1 = 1;
		double eps = m1 / k;
		double m2 = m1 - eps;

		// H
		double L0 = 2*m1 - eps;

		// N
		double N0 = 2*m1 - (m1-m2)/pow(2.0, double(k-2));
		double N1 = 0;
		for (int i = 0; i < k-1; i++) N1 += 1/pow(2.0, double(i));
		N1 *= m1;
		double L1 = Max(N0, N1);

		qDebug() << k << "\t\t" << n << "\t\t" << L0 <<"\t\t" << L1;
	}


}

HccGraph* Foldabilizer::activeHcc()
{
	return hccManager->activeHcc();
}

Q_EXPORT_PLUGIN(Foldabilizer)
