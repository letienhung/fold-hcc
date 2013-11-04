#include "foldem_widget.h"
#include "ui_foldem_widget.h"

foldem_widget::foldem_widget(Foldabilizer *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::foldem_widget)
{
    ui->setupUi(this);

	fold = f;

	// signal and slots
	fold->connect(ui->createL, SIGNAL(clicked()), SLOT(createL()));
	fold->connect(ui->createChair, SIGNAL(clicked()), SLOT(createChair()));
	fold->connect(ui->createT, SIGNAL(clicked()), SLOT(createT()));
	fold->connect(ui->createX, SIGNAL(clicked()), SLOT(createX()));
	fold->connect(ui->createU, SIGNAL(clicked()), SLOT(createU()));
	fold->connect(ui->load_hcc, SIGNAL(clicked()), SLOT(loadGraph()));
}


foldem_widget::~foldem_widget()
{
    delete ui;
}
