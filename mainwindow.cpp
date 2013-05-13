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
    dbContQuery= new QSqlQuery("CREATE table conn(idx INTEGER PRIMARY KEY ASC, name TEXT, databaseName TEXT  driver TEXT, user TEXT, pass TEXT, hostname TEXT, port INTEGER)",dbContainer);

    //Setup the model
    dbContModel = new EnhancedSqlTableModel(this);
    dbContModel->setTable("conn");
    dbContModel->select();
    dbContModel->setEditStrategy(QSqlTableModel::OnFieldChange);\
    dbContModel->removeColumns(3,3,QModelIndex());

    //Setup the table
    ui->tabServers->setModel(dbContModel);
    ui->tabServers->resizeColumnsToContents();
    ui->tabServers->hideColumn(0); //Hide, not remove, as not to ruin primary key.
    ui->tabServers->resizeColumnsToContents();
    ui->tabServers->resizeRowsToContents();
    ui->tabServers->setColumnWidth(1,240);


}

MainWindow::~MainWindow()
{
    delete ui;
}
