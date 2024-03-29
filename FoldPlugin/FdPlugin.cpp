#include "FdPlugin.h"
#include "FdWidget.h"
#include "ui_FdWidget.h"
#include "StarlabDrawArea.h"

#include "Graph.h"
#include <QDebug>
#include <QProcess>
#include "MeshHelper.h"
#include "PCA.h"
#include "ParSingleton.h"

#include <QFileInfo>
#include <QFileDialog>

// Helpful functor
struct CompareFdNode{
	bool operator()(ScaffNode* a, ScaffNode* b){ return a->mBox.Center.z() < b->mBox.Center.z(); }
};

FdPlugin::FdPlugin()
{
	widget = nullptr;

	// graph manager
	s_manager = new ScaffManager();
	this->connect(s_manager, SIGNAL(scaffoldChanged(Scaffold*)), SLOT(resetScene()));
	this->connect(s_manager, SIGNAL(scaffoldModified()), SLOT(updateScene()));
	this->connect(s_manager, SIGNAL(message(QString)), SLOT(showStatus(QString)));

	// fold manager
	f_manager = new FoldManager();
	f_manager->connect(s_manager, SIGNAL(scaffoldChanged(Scaffold*)), SLOT(setInputScaffold(Scaffold*)));
	this->connect(f_manager, SIGNAL(sceneChanged()), SLOT(updateScene()));
	this->connect(f_manager, SIGNAL(message(QString)), SLOT(showStatus(QString)));
	
	// visual tags
	drawNodeOrder = false;
	ParSingleton* ps = ParSingleton::instance();
	this->connect(ps, SIGNAL(visualOptionChanged()), SLOT(updateScene()));

	// color dialog
	qColorDialog = nullptr;
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

		// default settings
		ParSingleton* ps = ParSingleton::instance();
		widget->ui->showCuboid->setCheckState(ps->showCuboid ? Qt::Checked : Qt::Unchecked);
		widget->ui->showScaffold->setCheckState(ps->showScaffold ? Qt::Checked : Qt::Unchecked);
		widget->ui->showMesh->setCheckState(ps->showMesh ? Qt::Checked : Qt::Unchecked);
		widget->ui->showAABB->setCheckState(ps->showAABB ? Qt::Checked : Qt::Unchecked);
		widget->ui->showDecomp->setCheckState(ps->showDecomp ? Qt::Checked : Qt::Unchecked);
		widget->ui->showKeyframe->setCheckState(ps->showKeyframe ? Qt::Checked : Qt::Unchecked);

		Vector3 sqzV = ps->sqzV;
		if		(sqzV.x() ==  1) widget->ui->sqzV->setCurrentIndex(0);
		else if (sqzV.x() == -1) widget->ui->sqzV->setCurrentIndex(1);
		else if (sqzV.y() ==  1) widget->ui->sqzV->setCurrentIndex(2);
		else if (sqzV.y() == -1) widget->ui->sqzV->setCurrentIndex(3);
		else if (sqzV.z() ==  1) widget->ui->sqzV->setCurrentIndex(4);
		else if (sqzV.z() == -1) widget->ui->sqzV->setCurrentIndex(5);
		widget->ui->costWeight->setValue(ps->splitWeight);
		widget->ui->nbSplits->setValue(ps->maxNbSplits);
		widget->ui->nbChunks->setValue(ps->maxNbChunks);

		widget->ui->aabbX->setValue(ps->aabbCstrScale[0]);
		widget->ui->aabbY->setValue(ps->aabbCstrScale[1]);
		widget->ui->aabbZ->setValue(ps->aabbCstrScale[2]);

		widget->ui->nbKeyframes->setValue(ps->nbKeyframes);

		widget->ui->thickness->setValue(ps->thickness);
		widget->ui->connThrRatio->setValue(ps->connThrRatio);
	}

	// perspective
	drawArea()->setPerspectiveProjection();

	// connect to scaffold manager
	s_manager->connect(document(), SIGNAL(selectionChanged(Model*)), SLOT(setMesh(Model*)));
}

void FdPlugin::destroy()
{

}

void FdPlugin::decorate()
{
	if (activeScaffold())
	{
		// draw the active scaffold
		activeScaffold()->draw();

		// draw the order/depth of parts
		if( drawNodeOrder ){
			for(ScaffNode * n : activeScaffold()->getScaffNodes()){
				qglviewer::Vec center = drawArea()->camera()->projectedCoordinatesOf(qglviewer::Vec(n->center()));
				drawArea()->renderText( center.x, center.y, QString("[%1]").arg(activeScaffold()->nodes.indexOf(n)) );
			}
		}
	}
}

void FdPlugin::drawWithNames()
{
	if (activeScaffold())
		activeScaffold()->drawWithNames();
}

void FdPlugin::updateScene()
{
	drawArea()->updateGL();
}

void FdPlugin::resetScene()
{
	// show options
	ParSingleton* ps = ParSingleton::instance();
	ps->showDecomp = false;
	ps->showKeyframe = false;
	widget->ui->showDecomp->setCheckState(Qt::Unchecked);
	widget->ui->showKeyframe->setCheckState(Qt::Unchecked);
	
	// show the entire object
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

Scaffold* FdPlugin::activeScaffold()
{
	ParSingleton* ps = ParSingleton::instance();
	if (ps->showKeyframe) 
		return f_manager->getSelKeyframe();
	else if (ps->showDecomp)
		return f_manager->activeScaffold();
	else
		return s_manager->scaffold;
}

void FdPlugin::showStatus( QString msg )
{
	showMessage(msg.toStdString().c_str());
}

bool FdPlugin::postSelection( const QPoint& point )
{
	Q_UNUSED(point);

	Scaffold* activeFd = activeScaffold();
	if (activeFd)
	{
		int nidx = drawArea()->selectedName();

		ScaffNode* sn = (ScaffNode*)activeFd->getNode(nidx);
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
		if (qColorDialog && !selectedScaffNodes().isEmpty())
		{
			ScaffNode* selNode = selectedScaffNodes().front();
			qColorDialog->setCurrentColor(selNode->mColor);
		}
	}

	return true;
}


QVector<ScaffNode*> FdPlugin::selectedScaffNodes()
{
	QVector<ScaffNode*> selNodes;

	Scaffold* activeFd = activeScaffold();
	if (activeFd)
	{
		for(Structure::Node* n : activeFd->getSelectedNodes())
			selNodes << (ScaffNode*)n;
	}

	return selNodes;
}


void FdPlugin::exportCurrent()
{
	QString filename = QFileDialog::getSaveFileName(0, tr("Save Current Scaffold"), nullptr, tr("Mesh Files (*.obj)"));
	activeScaffold()->exportWholeMesh(filename);
	showMessage("Current mesh has been exported.");
}

void FdPlugin::exportAllObj()
{
	DecScaff* selDec = f_manager->shapeDec;
	if (!selDec) return;

	QString filename = QFileDialog::getSaveFileName(0, tr("Save Current Keyframes"), nullptr, tr("Mesh file (*.obj)"));
	QString basefilename = filename;
	basefilename.chop(4);

	for (int i = 0; i < selDec->keyframes.size(); i++)
	{	
		filename = basefilename + QString("%1").arg(QString::number(i), 3, '0') + ".obj";
		f_manager->selectKeyframe(i);
		selDec->getSelKeyframe()->exportWholeMesh(filename);
	}
}

#include "ChainScaff.h"
void FdPlugin::test1()
{
}

#include "HChainScaff.h"
void FdPlugin::test2()
{
	UnitScaff* selUnit = f_manager->getSelUnit();
	if (!selUnit) return;
	HChainScaff* hChain = (HChainScaff*)selUnit->getSelChain();
	FoldOption fo("", true, 1, 0, 1);
	hChain->applyFoldOption(&fo);
	//hChain->fold(0.1);
	updateScene();
}

void FdPlugin::showColorDialog()
{
	if(qColorDialog == nullptr)
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
	for(ScaffNode* n : selectedScaffNodes())
	{
		if (c.alpha() > 200) c.setAlpha(200);
		n->mColor = c;
	}

	updateScene();
}

void FdPlugin::saveSnapshot()
{
	Scaffold* activeFd = activeScaffold();
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
	
	DecScaff* shapeDec = f_manager->shapeDec;
	if (!shapeDec) return;

	QString filename = shapeDec->path + "/snapshot";
	drawArea()->setSnapshotFormat("PNG");
	drawArea()->setSnapshotQuality(100);
	drawArea()->setSnapshotFileName(filename);

	for (int i = 0; i < shapeDec->keyframes.size(); i++)
	{
		f_manager->selectKeyframe(i);
		updateScene();
		drawArea()->saveSnapshot(true, true);
	}
}

void FdPlugin::hideSelectedNodes()
{
	bool showKeyframe = ParSingleton::instance()->showKeyframe;
	QVector<ScaffNode*> snodes = selectedScaffNodes();
	for(ScaffNode* n : snodes)
	{
		n->isHidden = true;

		// hide this node on all key frames
		if (showKeyframe)
		{
			for(Scaffold* kf : f_manager->shapeDec->keyframes)
			{
				ScaffNode* kn = (ScaffNode*)kf->getNode(n->mID);
				kn->isHidden = true;
			}
		}
	}

	if( snodes.isEmpty() ){
		activeScaffold()->properties.clear();
		if (showKeyframe && f_manager->shapeDec) 
			for(Scaffold * scfd : f_manager->shapeDec->keyframes){
				scfd->properties.clear();
		}
	}

	updateScene();
}

void FdPlugin::unhideAllNodes()
{
	Scaffold* activeFd = activeScaffold();
	if (activeFd)
	{
		for(ScaffNode* n : activeFd->getScaffNodes())
			n->isHidden = false;
	}

	updateScene();
}

void FdPlugin::colorMasterSlave()
{
	Scaffold* activeFd = activeScaffold();
	if (activeFd)
	{
		for (ScaffNode* n : activeFd->getScaffNodes())
		{
			double grey = 240;
			QColor c = (!n->hasTag(MASTER_TAG)) ? 
				QColor::fromRgb(255, 110, 80) : QColor::fromRgb(grey, grey, grey);
			c.setAlphaF(0.78);
			n->mColor = c;
		}
	}

	updateScene();
}

void FdPlugin::exportPNG()
{
	DecScaff* shapeDec = f_manager->shapeDec;
	if (!shapeDec) return;

	ParSingleton::instance()->setShowKeyframe(true);

	QString filename = QFileDialog::getSaveFileName(0, tr("Save Current Scaffold"), nullptr, tr("PNG file (*.png)"));
	QString basefilename = filename;
	basefilename.chop(4);

	for (int i = 0; i < shapeDec->keyframes.size(); i++)
	{	
		filename = basefilename + QString("%1").arg(QString::number(i), 3, '0') + ".png";
		f_manager->selectKeyframe(i);
		updateScene();
		
		qApp->processEvents();
		drawArea()->grabFrameBuffer(true).save(filename);
		qApp->processEvents();
	}
}

void FdPlugin::exportSVG()
{
	int documentSize = 800;

	drawArea()->setMinimumSize(documentSize, documentSize);
	drawArea()->setMaximumSize(documentSize, documentSize);

	Scaffold* activeFd = activeScaffold();
	if (!activeFd) return;

	QVector<Scaffold *> activeFds;
	activeFds << activeFd;

	ParSingleton* ps = ParSingleton::instance();

	QString filename, basefilename;
	int ci = 0;
	if(widget->ui->isSVGsequence->isChecked())
	{
		activeFds.clear();

		// keyframes
		QVector<Scaffold*> selGraphs; 
		if (ps->showKeyframe && f_manager->shapeDec) selGraphs = f_manager->shapeDec->keyframes;
		for(Scaffold* g : selGraphs) activeFds << g;
	}

	for(Scaffold * afd : activeFds)
	{
		// output file
		if(filename.length() < 1) 
		{
			filename = QFileDialog::getSaveFileName(0, tr("Save Current Scaffold"), nullptr, tr("SVG file (*.svg)"));
			basefilename = filename;
			basefilename.chop(4);
		}

		// Sequence case
		if( widget->ui->isSVGsequence->isChecked() )
		{
			filename = basefilename + QString("%1").arg(QString::number(ci++), 3, '0') + ".svg";
		}

		// Export SVG
		{
			QFile file( filename );
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
			QTextStream out(&file);

			// SVG Header
			out << "<svg " + QString("width='%1' height='%2'").arg(documentSize).arg(documentSize) + " xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink'>\n";

			// Style
			QString style = "fill-opacity='0.78' stroke-width='0.25' stroke-linecap='round' stroke-linejoin='round' ";

			QVector<ScaffNode*> fdnodes = afd->getScaffNodes();

			if(ps->showCuboid)
			{
				std::sort( fdnodes.begin(), fdnodes.end(), CompareFdNode() );
			}
			else
			{
				std::reverse(fdnodes.begin(), fdnodes.end());
			}

			for (ScaffNode* n : fdnodes)
			{
				// skip hidden stuff for clean rendering
				if (n->hasTag(EDGE_VIRTUAL_TAG)) continue;

				if( ps->showScaffold )
				{
					if (n->mType == ScaffNode::PATCH)
					{
						/*// Edges
						for (Geom::Segment seg, ((PatchNode*)n)->mPatch.getEdgeSegments()){
							qglviewer::Vec proj0 = drawArea()->camera()->projectedCoordinatesOf( qglviewer::Vec(seg.P0) );
							qglviewer::Vec proj1 = drawArea()->camera()->projectedCoordinatesOf( qglviewer::Vec(seg.P1) );
							out << QString("<line x1='%1' y1='%2' x2='%3' y2='%4' style='%5' />").arg(proj0.x).arg(proj0.y).arg(proj1.x).arg(proj1.y).arg(style);
						}*/

						// Polygons
						out << QString("\n<polygon points='");
						for (Vector3 seg : ((PatchNode*)n)->mPatch.getConners()){
							qglviewer::Vec proj = drawArea()->camera()->projectedCoordinatesOf( qglviewer::Vec(seg) );
							out << QString("%1,%2 ").arg(proj.x).arg(proj.y);
						}
						out << QString("' %1/>\n").arg( style + QString("fill='%1' stroke='%2'").arg( n->mColor.name() ).arg( n->mColor.darker().name() ) );
					}
				}
				else if( ps->showCuboid )
				{
					out << "<g>\n";

					QVector<Geom::Rectangle> rects = n->mBox.getFaceRectangles();
					std::sort( rects.begin(), rects.end(), Geom::CompareRectangle() );
					for(Geom::Rectangle r : rects)
					{
						out << QString("\n<polygon points='");
						for (Vector3 p : r.getConners())
						{
							qglviewer::Vec proj = drawArea()->camera()->projectedCoordinatesOf( qglviewer::Vec(p) );
							out << QString("%1,%2 ").arg(proj.x).arg(proj.y);
						}
						out << QString("' %1/>\n").arg( style + QString("fill='%1' stroke='%2'").arg( n->mColor.name() ).arg( n->mColor.darker().name() ) );
					}

					out << "</g>\n";
				}
			}

			out << "\n</svg>";
		}

	}

    system( filename.toLatin1() );
}

bool FdPlugin::mousePressEvent(QMouseEvent* event)
{
	if (event->modifiers() & Qt::SHIFT)
	{
		drawArea()->select(event->pos());
		return true;
	}

	return false;
}

bool FdPlugin::keyPressEvent(QKeyEvent* event)
{
	// View save / restore
	if( event->modifiers().testFlag(Qt::ControlModifier) && 
		(event->key() == Qt::Key_W || event->key() == Qt::Key_E) )
	{
		drawArea()->setStateFileName("customcamera.xml");
		if( event->key() == Qt::Key_W ) drawArea()->saveStateToFile();
		else drawArea()->restoreStateFromFile();
		updateScene();
	}

	// Rearranging nodes
	if( event->modifiers().testFlag(Qt::ControlModifier) && 
		(event->key() == Qt::Key_F || event->key() == Qt::Key_B) )
	{
		QVector<Scaffold*> selGraphs;
		selGraphs << activeScaffold();

		QStringList selectedNodeNames;
		for(ScaffNode * n : selectedScaffNodes()) selectedNodeNames << n->mID;

		// We are changing keyframes
		ParSingleton* ps = ParSingleton::instance();
		if (ps->showKeyframe && f_manager->shapeDec) selGraphs = f_manager->shapeDec->keyframes;
		
		for(Scaffold* g : selGraphs)
		{
			drawNodeOrder = true;

			for(QString nid : selectedNodeNames)
			{
				Structure::Node * node = g->getNode(nid);
				int idx = g->nodes.indexOf(node);
				g->nodes.remove(idx);

				if( event->key() == Qt::Key_F )
					g->nodes.insert(std::max(0, idx - 1), node); // front
				else
					g->nodes.insert(std::min(idx + 1, g->nodes.size()), node); // back
			}
		}

		updateScene();

		return true;
	}

	// Change node color randomly
	if (event->modifiers().testFlag(Qt::ControlModifier) &&
		event->key() == Qt::Key_C)
	{
		for (ScaffNode* n : activeScaffold()->getScaffNodes())
		{
			n->setRandomColor();
		}

		updateScene();

		return true;
	}

	// merge selected nodes into bundle node
	// or un-group the single bundle node
	if (event->modifiers().testFlag(Qt::ControlModifier) &&
		event->key() == Qt::Key_G)
	{
		Scaffold* activeScaff = activeScaffold();
		QVector<ScaffNode*> selNodes = selectedScaffNodes();
		// unwrap: single bundle node will be unwrapped
		if (selNodes.size() == 1)
			activeScaff->unwrapBundleNode(selNodes.front()->mID);
		// wrap
		else activeScaff->wrapAsBundleNode(getIds(selNodes));

		updateScene();
		return true;
	}

	return false;
}