#include "foldem_widget.h"
#include "ui_foldem_widget.h"

foldem_widget::foldem_widget(foldabilizer *f, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::foldem_widget)
{
    ui->setupUi(this);

	plugin = f;

	// singnal and slots

}


foldem_widget::~foldem_widget()
{
    delete ui;
}
