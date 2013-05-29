#include "changedbs.h"
#include "ui_changedbs.h"

ChangeDBs::ChangeDBs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeDBs)
{
    ui->setupUi(this);

    drivers.removeAll("QMYSQL3");
    drivers.removeAll("QOCI8");
    drivers.removeAll("QODBC3");
    drivers.removeAll("QPSQL7");
    drivers.removeAll("QTDS7");
    drivers.removeAll("QSQLITE");

    ui->cmbDriver->addItems(drivers);

    //Customize fields for each driver type
    if(ui->cmbDriver->currentText() == "QODBC") this->setupPlaceholders("QODBC");       //One-item-only workaround
    connect(ui->cmbDriver,\
            SIGNAL(currentIndexChanged(QString)),\
            this,\
            SLOT(setupPlaceholders(QString)));

    //Buttons
    connect(ui->btnReset,SIGNAL(clicked()),this,SLOT(resetAction()));
    connect(ui->btnOk,SIGNAL(clicked()),this,SLOT(okAction()));
    connect(ui->btnCancel,SIGNAL(clicked()),this,SLOT(reject()));
}

ChangeDBs::~ChangeDBs()
{
    delete ui;
}


//Getters

QString ChangeDBs::name() const
{
    return this->ui->lnName->text();
}

QString ChangeDBs::dbName() const
{
    return this->ui->lnDbName->text();
}

QString ChangeDBs::userName() const
{
    return this->ui->lnUsername->text();
}

QString ChangeDBs::password() const
{
    QByteArray getBytes(this->ui->lnPassword->text().toUtf8(), this->ui->lnPassword->text().size());
    return QString(getBytes.toBase64());
}

QString ChangeDBs::hostName() const
{
    return this->ui->lnHostname->text();
}

QString ChangeDBs::driver() const
{
    return this->ui->cmbDriver->currentText();
}

int ChangeDBs::port() const
{
    return this->ui->lnPort->text().toInt();
}


//Setters

void ChangeDBs::setName(QString name)
{
    this->ui->lnName->setText(name);
}

void ChangeDBs::setDbName(QString dbName)
{
    this->ui->lnDbName->setText(dbName);
}

void ChangeDBs::setUserName(QString userName)
{
    this->ui->lnUsername->setText(userName);
}

void ChangeDBs::setPassword(QString password)
{
    QByteArray setBytes(password.toUtf8(), password.size());
    this->ui->lnPassword->setText(QString(QByteArray::fromBase64(setBytes)));
}

void ChangeDBs::setHostName(QString hostName)
{
    this->ui->lnHostname->setText(hostName);
}

void ChangeDBs::setDriver(QString driver)
{
    this->ui->cmbDriver->setCurrentText(driver);
}

void ChangeDBs::setPort(QString port)
{
    this->ui->lnPort->setText(port);
}

void ChangeDBs::resetAction()
{
    this->ui->cmbDriver->setCurrentIndex(0);
    this->ui->lnName->setText("");
    this->ui->lnDbName->setText("");
    this->ui->lnUsername->setText("");
    this->ui->lnPassword->setText("");
    this->ui->lnHostname->setText("");
    this->ui->lnPort->setText("");
}

void ChangeDBs::okAction()
{
}

void ChangeDBs::setupPlaceholders(QString driver)
{
    if (driver == "QODBC")
    {
        ui->lnUsername->setPlaceholderText("Username -- OPTIONAL");
        ui->lnPassword->setPlaceholderText("Password -- OPTIONAL");
        ui->lnDbName->setPlaceholderText("Database name / DSN");
    }
}
