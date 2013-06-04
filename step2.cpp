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

    ui->cmbFirstDB->setCurrentIndex(-1);
    ui->cmbSecondDB->setCurrentIndex(-1);

    //Remove offline databases
    connect(this->ui->cmbFirstDB,SIGNAL(currentIndexChanged(int)),SLOT(checkTableF(int)));
    connect(this->ui->cmbSecondDB,SIGNAL(currentIndexChanged(int)),SLOT(checkTableS(int)));
}

Step2::~Step2()
{
    testDbF.close();
    testDbF.removeDatabase("tests-F");
    testDbS.close();
    testDbS.removeDatabase("tests-S");
    delete ui;
}

void Step2::checkTableF(int row)
{    
    testDbF = QSqlDatabase::addDatabase(dbtCont->index(row,3).data().toString(),"tests-F");
    testDbF.setDatabaseName(dbtCont->index(row,2).data().toString());
    if(!dbtCont->index(row,4).data().toString().isEmpty())                              //Man-mode a.k.a manual DSN definition
        testDbF.setDatabaseName("Driver={"\
                               +dbtCont->index(row,4).data().toString()+"};DATABASE="\
                               +dbtCont->index(row,2).data().toString()+";");
    testDbF.setUserName(dbtCont->index(row,5).data().toString());
    testDbF.setPassword(dbtCont->index(row,6).data().toString());
    testDbF.setHostName(dbtCont->index(row,7).data().toString());
    testDbF.setPort(dbtCont->index(row,8).data().toInt());


    if(!testDbF.open())
    {
        dbtCont->setData(dbtCont->index(row,9),"DEAD");
        QMessageBox::warning(this,"DEAD","Connection failed with message:\n\n"+testDbF.lastError().text());
//        dbtCont->select();                                                              //Buggy select
    }

    QStandardItemModel *model = new QStandardItemModel(ui->tabFirstDB);
    model->insertColumns(0,2);
    model->insertRows(0,testDbF.tables().count());
    model->setHeaderData(0,Qt::Horizontal,"Table name", Qt::DisplayRole);
    model->setHeaderData(1,Qt::Horizontal,"Type", Qt::DisplayRole);

    for(int i=0;i<testDbF.tables().count();++i)
    {
        model->setData(model->index(i,0),testDbF.tables().at(i));
        model->setData(model->index(i,1),"BASE TABLE");
    }
    ui->tabFirstDB->setModel(model);
    ui->tabFirstDB->resizeColumnsToContents();
    ui->tabFirstDB->resizeRowsToContents();
    ui->tabFirstDB->setColumnWidth(0,ui->tabFirstDB->width()-ui->tabFirstDB->columnWidth(1));
    ui->tabFirstDB->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->tabFirstDB->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void Step2::checkTableS(int row)
{
    testDbS = QSqlDatabase::addDatabase(dbtCont->index(row,3).data().toString(),"tests-S");
    testDbS.setDatabaseName(dbtCont->index(row,2).data().toString());
    if(!dbtCont->index(row,4).data().toString().isEmpty())                              //Man-mode a.k.a manual DSN definition
        testDbS.setDatabaseName("Driver={"\
                               +dbtCont->index(row,4).data().toString()+"};DATABASE="\
                               +dbtCont->index(row,2).data().toString()+";");
    testDbS.setUserName(dbtCont->index(row,5).data().toString());
    testDbS.setPassword(dbtCont->index(row,6).data().toString());
    testDbS.setHostName(dbtCont->index(row,7).data().toString());
    testDbS.setPort(dbtCont->index(row,8).data().toInt());


    if(!testDbS.open())
    {
        dbtCont->setData(dbtCont->index(row,9),"DEAD");
        QMessageBox::warning(this,"DEAD","Connection failed with message:\n\n"+testDbS.lastError().text());
//        dbtCont->select();                                                              //Buggy select
    }

    QStandardItemModel *model = new QStandardItemModel(ui->tabSecondDB);
    model->insertColumns(0,2);
    model->insertRows(0,testDbS.tables().count());
    model->setHeaderData(0,Qt::Horizontal,"Table name", Qt::DisplayRole);
    model->setHeaderData(1,Qt::Horizontal,"Type", Qt::DisplayRole);

    for(int i=0;i<testDbS.tables().count();++i)
    {
        model->setData(model->index(i,0),testDbS.tables().at(i));
        model->setData(model->index(i,1),"BASE TABLE");
    }
    ui->tabSecondDB->setModel(model);
    ui->tabSecondDB->resizeColumnsToContents();
    ui->tabSecondDB->resizeRowsToContents();
    ui->tabSecondDB->setColumnWidth(0,ui->tabSecondDB->width()-ui->tabSecondDB->columnWidth(1));
    ui->tabSecondDB->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->tabSecondDB->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
