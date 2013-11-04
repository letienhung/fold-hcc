#include "foldem_widget.h"
#include "ui_foldem_widget.h"

foldem_widget::foldem_widget(Foldabilizer *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::foldem_widget)
{
    ui->setupUi(this);

	fold = f;

	// singnal and slots
	fold->connect(ui->createL, SIGNAL(clicked()), SLOT(createL()));
	fold->connect(ui->createChair, SIGNAL(clicked()), SLOT(createChair()));
}


foldem_widget::~foldem_widget()
{
    delete ui;
}
