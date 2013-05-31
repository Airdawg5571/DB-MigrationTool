#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Center window on screen
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, this->size(), qApp->desktop()->availableGeometry()));
    this->ui->statusBar->setSizeGripEnabled(false);

    /*********************************************************************
     *** Open/create SQLite file for stored database connection options
     *********************************************************************/

    dbContainer = QSqlDatabase::addDatabase("QSQLITE");
    dbContainer.setDatabaseName("../DB-MigrationTool/dbCont.sqlite");                   //TODO : Change this path to reflect the release path
    dbContainer.open();
    //Try to create table, if table already exists, will jump over with hidden SQLite error "Unable to execute statement"
    dbContQuery = new QSqlQuery("CREATE table conn(idx INTEGER PRIMARY KEY ASC, name TEXT, databaseName TEXT  driver TEXT, user TEXT, pass TEXT, hostname TEXT, port INTEGER, status TEXT)",dbContainer);

    //Setup the model
    dbContModel = new EnhancedSqlTableModel(this);
    dbContModel->setTable("conn");
    dbContModel->select();
    dbContModel->setHeaderData(1,Qt::Horizontal, "Name", Qt::DisplayRole);
    dbContModel->setHeaderData(2,Qt::Horizontal, "Database/DSN", Qt::DisplayRole);
    dbContModel->setHeaderData(7,Qt::Horizontal, "Host", Qt::DisplayRole);
    dbContModel->setHeaderData(8,Qt::Horizontal, "Port", Qt::DisplayRole);
    dbContModel->setHeaderData(9,Qt::Horizontal, "Status", Qt::DisplayRole);
    dbContModel->setEditStrategy(QSqlTableModel::OnFieldChange);\

    //"RESET" status column on "UNKNOWN"
    for (int i=0;i<dbContModel->rowCount(QModelIndex());i++)
        dbContModel->setData(dbContModel->index(i,9,QModelIndex()),"UNKNOWN",Qt::EditRole);

    //Setup the table
    ui->tabServers->setModel(dbContModel);
    ui->tabServers->hideColumn(0);                                                      //Hide, not remove, as not to ruin primary key.
    ui->tabServers->resizeColumnsToContents();
    ui->tabServers->resizeRowsToContents();
    ui->tabServers->setColumnWidth(1,240);
    ui->tabServers->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);   //Stretch first column to window
    ui->tabServers->hideColumn(3);                                                      //Don't show user/pass and driver
    ui->tabServers->hideColumn(4);
    ui->tabServers->hideColumn(5);
    ui->tabServers->hideColumn(6);


    /**************
     *** Buttons
     **************/

    //Add
    connect(ui->btnAdd,SIGNAL(clicked()),SLOT(addAction()));

    //Edit
    connect(ui->btnEdit,SIGNAL(clicked()),SLOT(editAction()));

    //Remove
    connect(ui->btnRemove,SIGNAL(clicked()),SLOT(removeAction()));

    //Check Status
    connect(ui->btnStatus,SIGNAL(clicked()),SLOT(checkAll()));

    //Make Edit and Remove respond to table selection
    connect(ui->tabServers,SIGNAL(clicked(QModelIndex)),this,SLOT(enableButtons(QModelIndex)));
    connect(ui->tabServers,SIGNAL(selectionCleared()),this,SLOT(disableButtons()));

    //Menu
    ui->tabServers->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tabServers,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(serversContextMenu(QPoint)));

    //Next
    connect(ui->cmdNextStep,SIGNAL(clicked()),SLOT(nextStep()));
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshServers()
{
    this->dbContModel->select();
    ui->tabServers->resizeColumnsToContents();
    ui->tabServers->resizeRowsToContents();
}

void MainWindow::enableButtons(QModelIndex index)
{
    if(index.isValid())
    {
        cellIndex = ui->tabServers->currentIndex();                                     // Save index first
        ui->btnEdit->setEnabled(true);
        ui->btnRemove->setEnabled(true);
    }
}

void MainWindow::disableButtons()
{
    ui->btnEdit->setEnabled(false);
    ui->btnRemove->setEnabled(false);
}

void MainWindow::serversContextMenu(const QPoint &pos)
{
    mnuServers = new QMenu();
    cellIndex = ui->tabServers->indexAt(pos);                                           // Save index first
    if(cellIndex.isValid())
    {
        mnuServers->addAction("Add new...", this, SLOT(addAction()));
        mnuServers->addAction("Edit item...", this, SLOT(editAction()));
        mnuServers->addAction("Remove item",this,SLOT(removeAction()));
        mnuServers->addSeparator();
        mnuServers->addAction("Check connectivity",this,SLOT(checkAction()));
        mnuServers->exec(ui->tabServers->mapToGlobal(pos));                             // Map menu
    }
}

void MainWindow::addAction()
{
    indexToBeEdited = "0";
    adddbs = new ChangeDBs();
    if(adddbs->exec() != ChangeDBs::Accepted)
        return;

    //Construct query
    QString query = "INSERT INTO conn (";
    query.append("name,");
    query.append("databasename,");
    query.append("driver");
    if (!adddbs->options().isEmpty())
        query.append(",option");
    if (!adddbs->userName().isEmpty())
        query.append(",user");
    if (!adddbs->password().isEmpty())
        query.append(",pass");
    if (!adddbs->hostName().isEmpty())
        query.append(",hostname");
    if (!adddbs->port().isEmpty())
        query.append(",port");
    query.append(") VALUES (");
    query.append("'"+adddbs->name()+"',");
    query.append("'"+adddbs->dbName()+"',");
    query.append("'"+adddbs->driver()+"'");
    if (!adddbs->options().isEmpty())
        query.append(",'"+adddbs->options()+"'");
    if (!adddbs->userName().isEmpty())
        query.append(",'"+adddbs->userName()+"'");
    if (!adddbs->password().isEmpty())
        query.append(",'"+adddbs->password()+"'");
    if (!adddbs->hostName().isEmpty())
        query.append(",'"+adddbs->hostName()+"'");
    if (!adddbs->port().isEmpty())
        query.append(",'"+adddbs->port()+"'");
    query.append(");");

    //Execute
    dbContQuery->clear();
    dbContQuery->exec(query);

    this->refreshServers();
}

void MainWindow::editAction()
{
    editdbs = new ChangeDBs(this);
    editdbs->setWindowTitle("Edit database...");
    //Populate
    indexToBeEdited = cellIndex.sibling(cellIndex.row(),0).data().toString();
    editdbs->setName(cellIndex.sibling(cellIndex.row(),1).data().toString());
    editdbs->setDbName(cellIndex.sibling(cellIndex.row(),2).data().toString());
    editdbs->setDriver(cellIndex.sibling(cellIndex.row(),3).data().toString());
    editdbs->setOptions(cellIndex.sibling(cellIndex.row(),4).data().toString());
    editdbs->setUserName(cellIndex.sibling(cellIndex.row(),5).data().toString());
    editdbs->setPassword(cellIndex.sibling(cellIndex.row(),6).data().toString());
    editdbs->setHostName(cellIndex.sibling(cellIndex.row(),7).data().toString());
    editdbs->setPort(cellIndex.sibling(cellIndex.row(),8).data().toString());
    if(!cellIndex.sibling(cellIndex.row(),4).data().toString().isEmpty()) editdbs->overrideDSN(true);
    if(editdbs->exec() != ChangeDBs::Accepted)
        return;

    //Update where needed
    dbContQuery->clear();
    dbContQuery->exec("UPDATE conn SET name = '"+editdbs->name()+"' WHERE idx = "+indexToBeEdited);
    dbContQuery->exec("UPDATE conn SET databaseName = '"+editdbs->dbName()+"' WHERE idx = "+indexToBeEdited);
    dbContQuery->exec("UPDATE conn SET driver = '"+editdbs->driver()+"' WHERE idx = "+indexToBeEdited);
    dbContQuery->exec("UPDATE conn SET option = '"+editdbs->options()+"' WHERE idx = "+indexToBeEdited);
    dbContQuery->exec("UPDATE conn SET user = '"+editdbs->userName()+"' WHERE idx = "+indexToBeEdited);
    dbContQuery->exec("UPDATE conn SET pass = '"+editdbs->password()+"' WHERE idx = "+indexToBeEdited);
    dbContQuery->exec("UPDATE conn SET hostname = '"+editdbs->hostName()+"' WHERE idx = "+indexToBeEdited);
    dbContQuery->exec("UPDATE conn SET port = '"+editdbs->port()+"' WHERE idx = "+indexToBeEdited);

    this->refreshServers();
}

void MainWindow::removeAction()
{
    this->dbContModel->removeRow(cellIndex.row());
    this->dbContModel->select();
}

void MainWindow::checkAction()
{
    testDb = QSqlDatabase::addDatabase(dbContModel->index(cellIndex.row(),3).data().toString(),"tests");
    testDb.setDatabaseName(dbContModel->index(cellIndex.row(),2).data().toString());
    if(!dbContModel->index(cellIndex.row(),4).data().toString().isEmpty())              //Man-mode a.k.a manual DSN definition
        testDb.setDatabaseName("Driver={"\
                               +dbContModel->index(cellIndex.row(),4).data().toString()+"};DATABASE="\
                               +dbContModel->index(cellIndex.row(),2).data().toString()+";");
    testDb.setUserName(dbContModel->index(cellIndex.row(),5).data().toString());
    testDb.setPassword(dbContModel->index(cellIndex.row(),6).data().toString());
    testDb.setHostName(dbContModel->index(cellIndex.row(),7).data().toString());
    testDb.setPort(dbContModel->index(cellIndex.row(),8).data().toInt());


    if(testDb.open())
        dbContModel->setData(dbContModel->index(cellIndex.row(),9),"ALIVE");
    else
    {
        dbContModel->setData(dbContModel->index(cellIndex.row(),9),"DEAD");
        QMessageBox::warning(this,"DEAD","Connection failed with message:\n\n"+testDb.lastError().text());
    }
    testDb.close();
    testDb.removeDatabase("tests");
    this->refreshServers();
}

void MainWindow::checkAll()
{
    for(int i=0;i<dbContModel->rowCount();i++)
    {
        cellIndex = dbContModel->index(i,0);
        this->checkAction();
    }
}

void MainWindow::nextStep()
{
    Step2 *secondStep = new Step2(NULL);
    secondStep->show();
    this->hide();
}
