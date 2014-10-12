#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QStringList>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), fmanager(NULL)
{
    ui->setupUi(this);
	ui->connThrRation->hide();

	// Load shapes
	{
		QString shapesfile = "shapes/shapes.txt";
		QFile file( shapesfile );
		file.open(QFile::ReadOnly | QFile::Text);
		QTextStream in(&file);
		QStringList lines = in.readAll().split('\n');

		foreach(QString line, lines)
		{
			QStringList shapeDetail = line.split("|");
			if( shapeDetail.size() < 6 ) continue;
			int c = 0;

			QString shapeFile = shapeDetail[c++];
			QString shapeName = QFileInfo(shapeFile).baseName();
			shapeParamters[ shapeName ]["shapeFile"] = shapeFile; 
			shapeParamters[ shapeName ]["shapeName"] = shapeName;
			shapeParamters[ shapeName ]["nSplits"] = shapeDetail[c++].toInt();
			shapeParamters[ shapeName ]["nChunks"] = shapeDetail[c++].toInt();
			shapeParamters[ shapeName ]["costWeight"] = shapeDetail[c++].toDouble();
			shapeParamters[ shapeName ]["threshold"] = shapeDetail[c++].toDouble();
			shapeParamters[ shapeName ]["newCost"] = shapeDetail[c++].toInt();
			shapeParamters[ shapeName ]["uniformHeight"] = shapeDetail[c++].toInt();
            shapeParamters[ shapeName ]["axis"] = shapeDetail[c++];

			// Populate list
			ui->shapesList->addItem( shapeName );
		}
	}

	// Connect list with loading of shapes
	this->connect( ui->shapesList, SIGNAL(currentIndexChanged(QString)), SLOT(loadShape(QString)) );
	if(shapeParamters.size()) loadShape( ui->shapesList->currentText() );

    // Connect foldablize button
    this->connect( ui->foldabilize, SIGNAL(clicked()), SLOT(foldabilize()));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::loadShape( QString shapeName )
{
	qDebug() << "Loading shape:" << shapeName;

	Scaffold * scaffold = new Scaffold();
	scaffold->loadFromFile( shapeParamters[ shapeName ]["shapeFile"].toString() );

	// Visual options
	scaffold->showCuboids( true );
	scaffold->showMeshes( false );
	scaffold->showScaffold( false );

	// Clean up
	if( fmanager ){
		ui->widget->setFoldManager( NULL );
		delete fmanager;
		fmanager = NULL;
	}

	fmanager = new FoldManager;
	fmanager->inputScaffold = scaffold;
	
	// Connect parameters and UI elements with manager
	{
		fmanager->setSqzV( shapeParamters[ shapeName ]["axis"].toString() );
		fmanager->connect(ui->nbSplits, SIGNAL(valueChanged(int)), SLOT(setNbSplits(int)));
		fmanager->connect(ui->nbChunks, SIGNAL(valueChanged(int)), SLOT(setNbChunks(int)));
		fmanager->connect(ui->connThrRation, SIGNAL(valueChanged(double)), SLOT(setConnThrRatio(double)));

		// Change UI
		{
			ui->nbSplits->setValue( shapeParamters[ shapeName ]["nSplits"].toInt() );
			ui->nbChunks->setValue( shapeParamters[ shapeName ]["nChunks"].toInt() );
			ui->costWeight->setValue( shapeParamters[ shapeName ]["costWeight"].toDouble() );
			ui->connThrRation->setValue( shapeParamters[ shapeName ]["threshold"].toDouble() );
		}

		fmanager->nbSplits = shapeParamters[ shapeName ]["nSplits"].toInt();
		fmanager->nbChunks = shapeParamters[ shapeName ]["nChunks"].toInt();
		fmanager->setThickness( 0.5 );
		fmanager->setNbKeyframes( 100 );
	}

	ui->widget->setFoldManager( fmanager );
}

void MainWindow::foldabilize()
{
	if( !fmanager ) return;

	ui->status->setText("Foldabilizing...");
	qApp->processEvents();

	// Set to wait cursor
	qApp->setOverrideCursor(Qt::WaitCursor);

	// Do foldabilize process
	fmanager->foldabilize();

	// Visual clean up
	{
		for(Scaffold * fgraph : fmanager->shapeDec->keyframes)
			fgraph->properties.clear();
		ui->widget->theta = 0.0;
	}

	// Ready to show keyframes
	fmanager->setProperty("isDone", true);

	ui->status->setText("Done.");
	qApp->processEvents();

	// Restore cursor
	qApp->restoreOverrideCursor();
	QCursor::setPos(QCursor::pos());
}
