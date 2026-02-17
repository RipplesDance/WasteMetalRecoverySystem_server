#include "quotationDialog.h"
#include "ui_quotationDialog.h"

quotationDialog::quotationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::quotationDialog)
{
    ui->setupUi(this);
}

quotationDialog::~quotationDialog()
{
    delete ui;
}
