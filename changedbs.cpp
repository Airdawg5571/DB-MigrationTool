#include "changedbs.h"
#include "ui_changedbs.h"

ChangeDBs::ChangeDBs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeDBs)
{
    ui->setupUi(this);
}

ChangeDBs::~ChangeDBs()
{
    delete ui;
}
