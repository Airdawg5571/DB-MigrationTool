#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Center window on screen
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, this->size(), qApp->desktop()->availableGeometry()));
    this->ui->statusBar->setSizeGripEnabled(false);

    /******************************************************************
     *** Open/create SQLite file for stored database connection options
     ******************************************************************/

    dbContainer = QSqlDatabase::addDatabase("QSQLITE");
    dbContainer.setDatabaseName("../DB-MigrationTool/dbCont.sqlite"); //TODO : Change this path to reflect the release path
    dbContainer.open();
    //Try to create table, if table already exists, will jump over with hidden SQLite error "Unable to execute statement"
    dbContQuery = new QSqlQuery("CREATE table conn(idx INTEGER PRIMARY KEY ASC, name TEXT, databaseName TEXT  driver TEXT, user TEXT, pass TEXT, hostname TEXT, port INTEGER, status TEXT)",dbContainer);

    //Setup the model
    dbContModel = new EnhancedSqlTableModel(this);
    dbContModel->setTable("conn");
    dbContModel->select();
    dbContModel->setHeaderData(1,Qt::Horizontal, "Name", Qt::DisplayRole);
    dbContModel->setHeaderData(2,Qt::Horizontal, "Database", Qt::DisplayRole);
    dbContModel->setHeaderData(3,Qt::Horizontal, "Host", Qt::DisplayRole);
    dbContModel->setHeaderData(4,Qt::Horizontal, "Port", Qt::DisplayRole);
    dbContModel->setHeaderData(5,Qt::Horizontal, "Status", Qt::DisplayRole);
    dbContModel->setEditStrategy(QSqlTableModel::OnFieldChange);\
    dbContModel->removeColumns(3,3,QModelIndex());

    //"RESET" status column on "UNKNOWN"
    for (int i=0;i<dbContModel->rowCount(QModelIndex());i++)
        dbContModel->setData(dbContModel->index(i,5,QModelIndex()),"UNKNOWN",Qt::EditRole);

    //Setup the table
    ui->tabServers->setModel(dbContModel);
    ui->tabServers->hideColumn(0); //Hide, not remove, as not to ruin primary key.
    ui->tabServers->resizeColumnsToContents();
    ui->tabServers->resizeRowsToContents();
    ui->tabServers->setColumnWidth(1,240);
    ui->tabServers->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch); //Stretch first column to window

    /******************************************************************
     ***
     ******************************************************************/



}

MainWindow::~MainWindow()
{
    delete ui;
}
