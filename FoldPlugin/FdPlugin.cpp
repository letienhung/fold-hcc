#include "FdPlugin.h"
#include "FdWidget.h"
#include "StarlabDrawArea.h"

#include "Graph.h"
#include <QDebug>
#include <QProcess>
#include "MeshHelper.h"
#include "PCA.h"

#include <QFileInfo>
#include <QFileDialog>

FdPlugin::FdPlugin()
{
	widget = NULL;

	// graph manager
	g_manager = new GraphManager();
	this->connect(g_manager, SIGNAL(scaffoldChanged(FdGraph*)), SLOT(resetScene()));
	this->connect(g_manager, SIGNAL(scaffoldModified()), SLOT(updateScene()));
	this->connect(g_manager, SIGNAL(message(QString)), SLOT(showStatus(QString)));

	// fold manager
	f_manager = new FoldManager();
	f_manager->connect(g_manager, SIGNAL(scaffoldChanged(FdGraph*)), SLOT(setScaffold(FdGraph*)));
	this->connect(f_manager, SIGNAL(sceneChanged()), SLOT(updateScene()));
	this->connect(f_manager, SIGNAL(message(QString)), SLOT(showStatus(QString)));
	
	// visual tags
	drawKeyframe = false;
	drawFolded = false;
	drawAABB = false;
	drawCuboid = true;
	drawScaffold = true;
	drawMesh = false;
	drawAFS = false;

	// color dialog
	qColorDialog = NULL;
}

void FdPlugin::create()
{
	if (!widget)
	{
		// create widget
        widget = new FdWidget(this);

		// locate 
		ModePluginDockWidget *dockwidget = new ModePluginDockWidget("Foldabilizer", mainWindow());
		dockwidget->setWidget(widget);
		mainWindow()->addDockWidget(Qt::RightDockWidgetArea, dockwidget);

		// perspective
		drawArea()->setPerspectiveProjection();
	}

	resetMesh();
}

void FdPlugin::destroy()
{

}

void FdPlugin::decorate()
{
	if (activeScaffold())
		activeScaffold()->draw();
}

void FdPlugin::drawWithNames()
{
	if (activeScaffold())
		activeScaffold()->drawWithNames();
}

void FdPlugin::updateScene()
{
	// update visual options
	FdGraph* active = activeScaffold();
	if (active)
	{
		active->showAABB = drawAABB;
		active->showCuboids(drawCuboid);
		active->showScaffold(drawScaffold);
		active->showMeshes(drawMesh);

		if (drawAFS) active->addTag(SHOW_AFS);
		else active->removeTag(SHOW_AFS);
	}

	drawArea()->updateGL();
}

void FdPlugin::resetScene()
{
	if (activeScaffold())
	{
		// adjust scene to show entire shape
		Geom::AABB aabb = activeScaffold()->computeAABB();
		aabb.validate();

		qglviewer::Vec bbmin(aabb.bbmin.data());
		qglviewer::Vec bbmax(aabb.bbmax.data());
		drawArea()->camera()->setSceneBoundingBox(bbmin, bbmax);
		drawArea()->camera()->showEntireScene();
	}
	
	// update visual options
	updateScene();
}

FdGraph* FdPlugin::activeScaffold()
{
	if (drawKeyframe) return f_manager->getSelKeyframe();

	return (drawFolded ? f_manager->activeScaffold() : g_manager->scaffold);
}

void FdPlugin::resetMesh()
{
	g_manager->setMesh(mesh());
}

void FdPlugin::showStatus( QString msg )
{
	showMessage(msg.toStdString().c_str());
}

bool FdPlugin::postSelection( const QPoint& point )
{
	Q_UNUSED(point);

	FdGraph* activeFd = activeScaffold();
	if (activeFd)
	{
		int nidx = drawArea()->selectedName();

		FdNode* sn = (FdNode*)activeFd->getNode(nidx);
		if (sn)
		{
			bool isSelected = activeFd->selectNode(nidx);
			if (isSelected)
				showMessage("Selected name = %d, nodeId = %s, mesh = %s", 
					nidx, qPrintable(sn->mID), qPrintable(sn->getMeshName()));
			else
				showMessage("Deselected name = %d, nodeId = %s", nidx, qPrintable(sn->mID));
		}
		else
		{
			activeFd->deselectAllNodes();
			showMessage("Deselected all.");
		}

		// set current color
		if (qColorDialog && !selectedFdNodes().isEmpty())
		{
			FdNode* selNode = selectedFdNodes().front();
			qColorDialog->setCurrentColor(selNode->mColor);
		}
	}

	return true;
}


QVector<FdNode*> FdPlugin::selectedFdNodes()
{
	QVector<FdNode*> selNodes;

	FdGraph* activeFd = activeScaffold();
	if (activeFd)
	{
		foreach(Structure::Node* n, activeFd->getSelectedNodes())
			selNodes << (FdNode*)n;
	}

	return selNodes;
}



void FdPlugin::showFolded( int state )
{
	drawFolded = (state == Qt::Checked);
	updateScene();
}

void FdPlugin::showAABB( int state )
{
	drawAABB = (state == Qt::Checked);
	updateScene();
}

void FdPlugin::showCuboid( int state )
{
	drawCuboid = (state == Qt::Checked);
	updateScene();
}

void FdPlugin::showScaffold( int state )
{
	drawScaffold = (state == Qt::Checked);
	updateScene();
}

void FdPlugin::showMesh( int state )
{
	drawMesh = (state == Qt::Checked);
	updateScene();
}

void FdPlugin::showKeyframe( int state )
{
	drawKeyframe = (state == Qt::Checked);
	updateScene();
}


void FdPlugin::showAFS( int state )
{
	drawAFS = (state == Qt::Checked);
	updateScene();
}


void FdPlugin::exportCurrent()
{
	QString filename = QFileDialog::getSaveFileName(0, tr("Save Current Scaffold"), NULL, tr("Mesh Files (*.obj)"));
	activeScaffold()->exportMesh(filename);
	showMessage("Current mesh has been exported.");
}

#include "ChainGraph.h"
void FdPlugin::test1()
{
	BlockGraph* selBlk = f_manager->getSelBlock();
	if (selBlk)
	{
		FoldOption fn(true, 1, 0, 1, "hhh");
		double thickness = 0;
		foreach (ChainGraph* chain, selBlk->chains)
		{
			chain->halfThk = thickness * 0.5;
			chain->baseOffset = thickness * 0.5;
			//chain->topMaster->setThickness(thickness);
			//chain->baseMaster->setThickness(thickness);
			//chain->getFoldRegion(&fn);
			chain->applyFoldOption(&fn);
			chain->fold(0);
		}
	}
}

void FdPlugin::test2()
{
}

void FdPlugin::showColorDialog()
{
	if(qColorDialog == NULL)
	{
		// create
		qColorDialog = new QColorDialog();
		qColorDialog->hide();
		qColorDialog->setOption(QColorDialog::ShowAlphaChannel, true);
		qColorDialog->setOption(QColorDialog::DontUseNativeDialog,false);
		qColorDialog->setOption(QColorDialog::NoButtons,true);
		qColorDialog->setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
		connect(mainWindow(), SIGNAL(destroyed()), qColorDialog, SLOT(deleteLater()));

		// Predefined colors
		QVector<QColor> selColors;
		selColors << QColor::fromRgb(71, 92, 244)	// blue
				  << QColor::fromRgb(129, 168, 48)	// yellow
				  << QColor::fromRgb(240, 43, 82)	// red
				  << QColor::fromRgb(205, 104, 234)	// purple - pink
				  << QColor::fromRgb(93, 54, 192)	// purple 
				  << QColor::fromRgb(234, 67, 16)	// orange
				  << QColor::fromRgb(54, 145, 89)	// green-light
				  << QColor::fromRgb(36, 93, 16)	// green
				  << QColor::fromRgb(44, 201, 212)	// cyan
				  << QColor::fromRgb(255, 93, 109);	// pink
		for (int i = 0; i < selColors.size(); i++) 
			qColorDialog->setCustomColor(i, selColors[i].rgb());

		// signals
		this->connect(qColorDialog, SIGNAL(currentColorChanged(QColor)), SLOT(updateSelNodesColor(QColor)));
	}

	qColorDialog->show();
}

void FdPlugin::updateSelNodesColor( QColor c )
{
	foreach (FdNode* n, selectedFdNodes())
	{
		if (c.alpha() > 200) c.setAlpha(200);
		n->mColor = c;
	}

	updateScene();
}

void FdPlugin::saveSnapshot()
{
	FdGraph* activeFd = activeScaffold();
	if (!activeFd) return;

	QString path = QFileInfo(activeFd->path).absolutePath();
	QString filename = path  + "/" + activeFd->mID + "_snapshot";
	drawArea()->setSnapshotFormat("PNG");
	drawArea()->setSnapshotQuality(100);
	drawArea()->setSnapshotFileName(filename);
	drawArea()->saveSnapshot(true, true);
}

void FdPlugin::saveSnapshotAll()
{
	if (!f_manager) return;
	
	DcGraph* selDc = f_manager->getSelDcGraph();
	if (!selDc) return;

	drawKeyframe = true;
	QString filename = selDc->path + "/snapshot";
	drawArea()->setSnapshotFormat("PNG");
	drawArea()->setSnapshotQuality(100);
	drawArea()->setSnapshotFileName(filename);

	for (int i = 0; i < selDc->keyframes.size(); i++)
	{
		f_manager->selectKeyframe(i);
		updateScene();
		drawArea()->saveSnapshot(true, true);
	}
}

void FdPlugin::hideSelectedNodes()
{
	foreach (FdNode* n, selectedFdNodes())
		n->isHidden = true;

	updateScene();
}

void FdPlugin::unhideAllNodes()
{
	FdGraph* activeFd = activeScaffold();
	if (activeFd)
	{
		foreach (FdNode* n, activeFd->getFdNodes())
			n->isHidden = false;
	}

	updateScene();
}

void FdPlugin::hideEdgeRods()
{
	FdGraph* activeFd = activeScaffold();
	if (activeFd) activeFd->hideEdgeRods();
	updateScene();
}

void FdPlugin::colorMasterSlave()
{
	FdGraph* activeFd = activeScaffold();
	if (activeFd)
	{
		foreach (FdNode* n, activeFd->getFdNodes())
		{
			double grey = 240;
			QColor c = (n->hasTag(MASTER_TAG)) ? 
				QColor::fromRgb(255, 110, 80) : QColor::fromRgb(grey, grey, grey);
			c.setAlphaF(0.78);
			n->mColor = c;
		}
	}

	updateScene();
}

void FdPlugin::exportSVG()
{
	QVector<Geom::Segment> segs;
	FdGraph* activeFd = activeScaffold();
	if (!activeFd) return;

	// SVG output file
	QString filename = QFileDialog::getSaveFileName(0, tr("Save Current Scaffold"), NULL, tr("SVG file (*.svg)"));
	
	QFile file( filename );
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
	QTextStream out(&file);

	// SVG Header
	out << "<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink'>\n";

	// Style
	QString style = "stroke:#660000; fill:#FFAAFF; fill-opacity: 0.75; stroke-width: 1; stroke-miterlimit: round";

	foreach (FdNode* n, activeFd->getFdNodes()){
		if (n->mType == FdNode::PATCH)
		{
			/*// Edges
			foreach (Geom::Segment seg, ((PatchNode*)n)->mPatch.getEdgeSegments()){
				qglviewer::Vec proj0 = drawArea()->camera()->projectedCoordinatesOf( qglviewer::Vec(seg.P0) );
				qglviewer::Vec proj1 = drawArea()->camera()->projectedCoordinatesOf( qglviewer::Vec(seg.P1) );
				out << QString("<line x1='%1' y1='%2' x2='%3' y2='%4' style='%5' />").arg(proj0.x).arg(proj0.y).arg(proj1.x).arg(proj1.y).arg(style);
			}*/

			// Polygons
			out << QString("\n<polygon points='");
			foreach (Vector3 seg, ((PatchNode*)n)->mPatch.getConners()){
				qglviewer::Vec proj = drawArea()->camera()->projectedCoordinatesOf( qglviewer::Vec(seg) );
				out << QString("%1,%2 ").arg(proj.x).arg(proj.y);
			}
			out << QString("' style='%1'/>\n").arg( style );
		}
	}

	out << "\n</svg>";
}

Q_EXPORT_PLUGIN(FdPlugin)