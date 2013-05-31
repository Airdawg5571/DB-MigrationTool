#include "step2.h"
#include "ui_step2.h"
#include <QDebug>

Step2::Step2(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Step2)
{
    ui->setupUi(this);

    //Center window on screen
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, this->size(), qApp->desktop()->availableGeometry()));
    this->ui->statusBar->setSizeGripEnabled(false);

    /*********************************************************************
     *** Open/create SQLite file for stored database connection options
     *********************************************************************/
    dbContainer = QSqlDatabase::addDatabase("QSQLITE", "dbContainer");
    dbContainer.setDatabaseName("../DB-MigrationTool/dbCont.sqlite");                   //TODO : Change this path to reflect the release path
    dbContainer.open();

    //DB init
    dbtCont = new EnhancedSqlTableModel(this);
    dbtCont->setTable("conn");
    dbtCont->setEditStrategy(QSqlTableModel::OnManualSubmit);
    dbtCont->setFilter("status != 'DEAD'");
    dbtCont->select();

    ui->cmbFirstDB->setModel(dbtCont);
    ui->cmbSecondDB->setModel(dbtCont);

    ui->cmbFirstDB->setModelColumn(1);
    ui->cmbSecondDB->setModelColumn(1);

    //Remove offline databases
    connect(this->ui->cmbFirstDB,SIGNAL(currentIndexChanged(int)),SLOT(checkTables(int)));
    connect(this->ui->cmbSecondDB,SIGNAL(currentIndexChanged(int)),SLOT(checkTables(int)));
}

Step2::~Step2()
{
    delete ui;
}

void Step2::checkTables(int row)
{    
    testDb = QSqlDatabase::addDatabase(dbtCont->index(row,3).data().toString(),"tests");
    testDb.setDatabaseName(dbtCont->index(row,2).data().toString());
    if(!dbtCont->index(row,4).data().toString().isEmpty())              //Man-mode a.k.a manual DSN definition
        testDb.setDatabaseName("Driver={"\
                               +dbtCont->index(row,4).data().toString()+"};DATABASE="\
                               +dbtCont->index(row,2).data().toString()+";");
    testDb.setUserName(dbtCont->index(row,5).data().toString());
    testDb.setPassword(dbtCont->index(row,6).data().toString());
    testDb.setHostName(dbtCont->index(row,7).data().toString());
    testDb.setPort(dbtCont->index(row,8).data().toInt());


    if(!testDb.open())
    {
        dbtCont->setData(dbtCont->index(row,9),"DEAD");
        QMessageBox::warning(this,"DEAD","Connection failed with message:\n\n"+testDb.lastError().text());
        dbtCont->select();
    }
    testDb.close();
    testDb.removeDatabase("tests");
}
