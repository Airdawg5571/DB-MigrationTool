#include "changedbs.h"
#include "ui_changedbs.h"
#include <QDebug>

ChangeDBs::ChangeDBs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeDBs)
{
    ui->setupUi(this);
    drivers = QSqlDatabase::drivers();
    odbcDrivers = QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers",QSettings::NativeFormat).childKeys();

    //Unimplemented QtSql drivers - no plugins
    drivers.removeAll("QMYSQL3");
    drivers.removeAll("QOCI8");
    drivers.removeAll("QODBC3");
    drivers.removeAll("QPSQL7");
    drivers.removeAll("QTDS7");
    drivers.removeAll("QSQLITE");

    //"File DSN" ODBC drivers - useless
    odbcDrivers.removeAll("Driver da Microsoft para arquivos texto (*.txt; *.csv)");
    odbcDrivers.removeAll("Driver do Microsoft Access (*.mdb)" );
    odbcDrivers.removeAll("Driver do Microsoft Excel(*.xls)" );
    odbcDrivers.removeAll("Driver do Microsoft Paradox (*.db )");
    odbcDrivers.removeAll("Driver do Microsoft dBase (*.dbf)" );
    odbcDrivers.removeAll("Driver para o Microsoft Visual FoxPro" );
    odbcDrivers.removeAll("Microsoft Access Driver (*.mdb)" );
    odbcDrivers.removeAll("Microsoft Access-Treiber (*.mdb)" );
    odbcDrivers.removeAll("Microsoft Excel Driver (*.xls)" );
    odbcDrivers.removeAll("Microsoft Excel-Treiber (*.xls)" );
    odbcDrivers.removeAll("Microsoft FoxPro VFP Driver (*.dbf)" );
    odbcDrivers.removeAll("Microsoft Paradox Driver (*.db )" );
    odbcDrivers.removeAll("Microsoft Paradox-Treiber (*.db )" );
    odbcDrivers.removeAll("Microsoft Text Driver (*.txt; *.csv)" );
    odbcDrivers.removeAll("Microsoft Text-Treiber (*.txt; *.csv)" );
    odbcDrivers.removeAll("Microsoft Visual FoxPro-Treiber" );
    odbcDrivers.removeAll("Microsoft dBase Driver (*.dbf)" );
    odbcDrivers.removeAll("Microsoft dBase VFP Driver (*.dbf)" );
    odbcDrivers.removeAll("Microsoft dBase-Treiber (*.dbf)" );

    ui->cmbDriver->addItems(drivers);
    ui->cmbODBC->addItems(odbcDrivers);
    ui->cmbODBC->setVisible(false);
    ui->chkDSN->setVisible(false);
    ui->chkDSN->setChecked(true);                                                       //Check it so you can uncheck it 15 lines below :|

    //Customize fields for each driver type
    if(ui->cmbDriver->currentText() == "QODBC") this->setupLayout("QODBC");             //One-item-only workaround
    connect(ui->cmbDriver,\
            SIGNAL(currentIndexChanged(QString)),\
            this,\
            SLOT(setupLayout(QString)));

    //Buttons
    connect(ui->btnClear,SIGNAL(clicked()),this,SLOT(clearAction()));
    connect(ui->btnOk,SIGNAL(clicked()),this,SLOT(okAction()));
    connect(ui->btnCancel,SIGNAL(clicked()),this,SLOT(reject()));
    connect(ui->btnTest,SIGNAL(clicked()),this,SLOT(testAction()));

    connect(ui->chkDSN,SIGNAL(toggled(bool)),SLOT(overrideDSN(bool)));
    ui->chkDSN->setChecked(false);
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

QString ChangeDBs::options() const
{
    if (ui->chkDSN->isChecked()) return this->ui->cmbODBC->currentText();
    else return QString("");
}

QString ChangeDBs::port() const
{
    return this->ui->lnPort->text();
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

void ChangeDBs::setOptions(QString option)
{
    this->ui->cmbODBC->setCurrentText(option);
}

void ChangeDBs::setPort(QString port)
{
    this->ui->lnPort->setText(port);
}

void ChangeDBs::clearAction()
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
    //TODO: Add validation and sanitize input
    accept();
}

void ChangeDBs::testAction()
{
    testDb = QSqlDatabase::addDatabase(driver(),"tests-chdb");
    testDb.setDatabaseName(dbName());
    if(ui->chkDSN->isChecked())                                                        //Man-mode a.k.a manual DSN definition
        testDb.setDatabaseName("Driver={"\
                               +options()+"};DATABASE="\
                               +dbName()+";");
    testDb.setUserName(userName());
    testDb.setPassword(password());
    testDb.setHostName(hostName());
    testDb.setPort(port().toInt());


    if(testDb.open())
        QMessageBox::information(this,"ALIVE","Connection successful!");
    else
        QMessageBox::warning(this,"DEAD","Connection failed with message:\n\n"+testDb.lastError().text());
    testDb.close();
    testDb.removeDatabase("tests-chdb");
}

void ChangeDBs::setupLayout(QString driver)
{
    if (driver == "QODBC")
    {
        ui->lnUsername->setPlaceholderText("Username");
        ui->lnPassword->setPlaceholderText("Password");
        ui->lnDbName->setPlaceholderText("DSN name");
        ui->chkDSN->setVisible(true);
        ui->cmbODBC->setCurrentIndex(2);                                                //Set on MySQL for now...
    }
}

void ChangeDBs::overrideDSN(bool ovr)                                                   //Hide/show options already in DSN
{
    if(ovr)
    {
        ui->chkDSN->setChecked(true);
        ui->lnDbName->setPlaceholderText("Database name");
        ui->cmbODBC->show();
        ui->lnHostname->show();
        ui->lnPort->show();
        ui->lnUsername->show();
        ui->lnPassword->show();
    }
    else
    {
        ui->lnDbName->setPlaceholderText("Data source name (DSN)");
        ui->cmbODBC->hide();
        ui->lnHostname->hide();
        ui->lnPort->hide();
        ui->lnUsername->hide();
        ui->lnPassword->hide();
    }

}
