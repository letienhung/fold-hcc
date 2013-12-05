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

}

HccGraph* Foldabilizer::activeHcc()
{
	return hccManager->activeHcc();
}

Q_EXPORT_PLUGIN(Foldabilizer)
