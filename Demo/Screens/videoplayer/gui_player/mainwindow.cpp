#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

#include "VideoWidget.h"
#include "VideoToolbar.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    VideoWidget * v = new VideoWidget();
    ui->centralLayout->addWidget(v);

    VideoToolbar * t = new VideoToolbar();
    ui->centralLayout->addWidget(t);

    connect(t, SIGNAL(valueChanged(int)), v, SLOT(seekVideo(int)));
    connect(t->ui->playButton, SIGNAL(clicked()), v, SLOT(togglePlay()));
    connect(v, SIGNAL(setSliderValue(int)), t->ui->slider, SLOT(setValue(int)));

    QString fileName = QFileDialog::getOpenFileName(this, "Open Video", "", "Video Files (*.ogg)");

    v->loadVideo(fileName);
}

MainWindow::~MainWindow()
{
    delete ui;
}
