#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_indexToBeEdited(QStringLiteral("0"))
{
    ui->setupUi(this);

    //Center window on screen
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter, this->size(), qApp->desktop()->availableGeometry()));
    this->ui->statusBar->setSizeGripEnabled(false);

    /*********************************************************************
     *** Open/create SQLite file for stored database connection options
     *********************************************************************/

    m_dbContainer = QSqlDatabase::addDatabase("QSQLITE");
    m_dbContainer.setDatabaseName("../DB-MigrationTool/dbCont.sqlite");                   //TODO : Change this path to reflect the release path
    m_dbContainer.open();
    //Try to create table, if table already exists, will jump over with hidden SQLite error "Unable to execute statement"
    QSqlQuery("CREATE TABLE IF NOT EXISTS conn(idx INTEGER PRIMARY KEY ASC, name TEXT, databaseName TEXT  driver TEXT, user TEXT, pass TEXT, hostname TEXT, port INTEGER, status TEXT)", m_dbContainer);


    //Setup the model
    m_dbContModel = new EnhancedSqlTableModel(this);
    m_dbContModel->setTable("conn");
    m_dbContModel->select();
    m_dbContModel->setHeaderData(1,Qt::Horizontal, "Name", Qt::DisplayRole);
    m_dbContModel->setHeaderData(2,Qt::Horizontal, "Database/DSN", Qt::DisplayRole);
    m_dbContModel->setHeaderData(7,Qt::Horizontal, "Host", Qt::DisplayRole);
    m_dbContModel->setHeaderData(8,Qt::Horizontal, "Port", Qt::DisplayRole);
    m_dbContModel->setHeaderData(9,Qt::Horizontal, "Status", Qt::DisplayRole);
    m_dbContModel->setEditStrategy(QSqlTableModel::OnFieldChange);\

    //"RESET" status column on "UNKNOWN"
    for (int i=0;i<m_dbContModel->rowCount(QModelIndex());i++)
        m_dbContModel->setData(m_dbContModel->index(i,9,QModelIndex()),"UNKNOWN",Qt::EditRole);

    //Setup the table
    ui->tabServers->setModel(m_dbContModel);
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
    this->m_dbContModel->select();
    ui->tabServers->resizeColumnsToContents();
    ui->tabServers->resizeRowsToContents();
}

void MainWindow::enableButtons(QModelIndex index)                                       //Enable buttons if there's a valid index
{
    if(index.isValid())
    {
        m_cellIndex = ui->tabServers->currentIndex();                                     //Save index first
        ui->btnEdit->setEnabled(true);
        ui->btnRemove->setEnabled(true);
    }
}

void MainWindow::disableButtons()                                                       //...Else disable them
{
    ui->btnEdit->setEnabled(false);
    ui->btnRemove->setEnabled(false);
}

void MainWindow::serversContextMenu(const QPoint &pos)                                  //Context menu for the table
{
    m_mnuServers = new QMenu();
    m_cellIndex = ui->tabServers->indexAt(pos);                                           //Save index first
    if(m_cellIndex.isValid())
    {
        m_mnuServers->addAction("Add new...", this, SLOT(addAction()));
        m_mnuServers->addAction("Edit item...", this, SLOT(editAction()));
        m_mnuServers->addAction("Remove item",this,SLOT(removeAction()));
        m_mnuServers->addSeparator();
        m_mnuServers->addAction("Check connectivity",this,SLOT(checkAction()));
        m_mnuServers->exec(ui->tabServers->mapToGlobal(pos));                             //Map menu
    }
}

void MainWindow::addAction()                                                            //"Add" action for button and context-menu
{                                                                                       // ^- self explanatory
    m_indexToBeEdited = "0";
    m_adddbs = new ChangeDBs();
    if(m_adddbs->exec() != ChangeDBs::Accepted)
        return;


    // TODO: prepare
    //Construct query
    QString query = "INSERT INTO conn (";
    query.append("name,");
    query.append("databasename,");
    query.append("driver");
    if (!m_adddbs->options().isEmpty())
        query.append(",option");
    if (!m_adddbs->userName().isEmpty())
        query.append(",user");
    if (!m_adddbs->password().isEmpty())
        query.append(",pass");
    if (!m_adddbs->hostName().isEmpty())
        query.append(",hostname");
    if (!m_adddbs->port().isEmpty())
        query.append(",port");
    query.append(") VALUES (");
    query.append("'"+m_adddbs->name()+"',");
    query.append("'"+m_adddbs->dbName()+"',");
    query.append("'"+m_adddbs->driver()+"'");
    if (!m_adddbs->options().isEmpty())
        query.append(",'"+m_adddbs->options()+"'");
    if (!m_adddbs->userName().isEmpty())
        query.append(",'"+m_adddbs->userName()+"'");
    if (!m_adddbs->password().isEmpty())
        query.append(",'"+m_adddbs->password()+"'");
    if (!m_adddbs->hostName().isEmpty())
        query.append(",'"+m_adddbs->hostName()+"'");
    if (!m_adddbs->port().isEmpty())
        query.append(",'"+m_adddbs->port()+"'");
    query.append(");");


    //Execute
    m_dbContQuery->clear();
    m_dbContQuery->exec(query);

    for (auto x, list)
    {m_dbContQuery.addb

    this->refreshServers();
}

void MainWindow::editAction()                                                           //Same as "Add" but with prefilled form
{
    m_editdbs = new ChangeDBs(this);
    m_editdbs->setWindowTitle("Edit database...");
    //Populate
    m_indexToBeEdited = m_cellIndex.sibling(m_cellIndex.row(),0).data().toString();
    m_editdbs->setName(m_cellIndex.sibling(m_cellIndex.row(),1).data().toString());
    m_editdbs->setDbName(m_cellIndex.sibling(m_cellIndex.row(),2).data().toString());
    m_editdbs->setDriver(m_cellIndex.sibling(m_cellIndex.row(),3).data().toString());
    m_editdbs->setOptions(m_cellIndex.sibling(m_cellIndex.row(),4).data().toString());
    m_editdbs->setUserName(m_cellIndex.sibling(m_cellIndex.row(),5).data().toString());
    m_editdbs->setPassword(m_cellIndex.sibling(m_cellIndex.row(),6).data().toString());
    m_editdbs->setHostName(m_cellIndex.sibling(m_cellIndex.row(),7).data().toString());
    m_editdbs->setPort(m_cellIndex.sibling(m_cellIndex.row(),8).data().toString());
    if(!m_cellIndex.sibling(m_cellIndex.row(),4).data().toString().isEmpty()) m_editdbs->overrideDSN(true);
    if(m_editdbs->exec() != ChangeDBs::Accepted)
        return;

    //Update where needed
    m_dbContQuery->clear();
    bool ok = m_dbContQuery->prepare("UPDATE conn\n"
                           "SET name = ?\n"
                           "SET databaseName = ?\n"
                           "SET driver = ?\n"
                           "SET option = ?\n"
                           "SET user = ?\n"
                           "SET pass = ?\n"
                           "SET hostname = ?\n"
                           "SET port = ?\n"
                           "WHERE idx = ?\n"
                           );

    Q_ASSERT(ok);

    m_dbContQuery->addBindValue(m_editdbs->name());
    m_dbContQuery->addBindValue(m_editdbs->dbName());
    m_dbContQuery->addBindValue(m_editdbs->driver());
    m_dbContQuery->addBindValue(m_editdbs->options());
    m_dbContQuery->addBindValue(m_editdbs->userName());
    m_dbContQuery->addBindValue(m_editdbs->password());
    m_dbContQuery->addBindValue(m_editdbs->hostName());
    m_dbContQuery->addBindValue(m_editdbs->port());
    m_dbContQuery->addBindValue(m_indexToBeEdited);

    ok = m_dbContQuery->exec();

    Q_ASSERT(ok);

//    m_dbContQuery->exec("UPDATE conn SET name = '"+m_editdbs->name()+"' WHERE idx = "+m_indexToBeEdited);
//    m_dbContQuery->exec("UPDATE conn SET databaseName = '"+m_editdbs->dbName()+"' WHERE idx = "+m_indexToBeEdited);
//    m_dbContQuery->exec("UPDATE conn SET driver = '"+m_editdbs->driver()+"' WHERE idx = "+m_indexToBeEdited);
//    m_dbContQuery->exec("UPDATE conn SET option = '"+m_editdbs->options()+"' WHERE idx = "+m_indexToBeEdited);
//    m_dbContQuery->exec("UPDATE conn SET user = '"+m_editdbs->userName()+"' WHERE idx = "+m_indexToBeEdited);
//    m_dbContQuery->exec("UPDATE conn SET pass = '"+m_editdbs->password()+"' WHERE idx = "+m_indexToBeEdited);
//    m_dbContQuery->exec("UPDATE conn SET hostname = '"+m_editdbs->hostName()+"' WHERE idx = "+m_indexToBeEdited);
//    m_dbContQuery->exec("UPDATE conn SET port = '"+m_editdbs->port()+"' WHERE idx = "+m_indexToBeEdited);


    this->refreshServers();
}

void MainWindow::removeAction()                                                         //DROP
{
    this->m_dbContModel->removeRow(m_cellIndex.row());
    this->m_dbContModel->select();
}

void MainWindow::checkAction()                                                          //Check status of selected server
{
    //Define a database with credentials from the table
    m_testDb = QSqlDatabase::addDatabase(m_dbContModel->index(m_cellIndex.row(),3).data().toString(),"tests");

    auto list = m_dbContModel->getValuesAt(m_cellIndex.row(), { 2, 4, 5, 6, 7, 8 });

    m_testDb.setDatabaseName(list[0].toString());
    if(!m_dbContModel->index(m_cellIndex.row(),4).data().toString().isEmpty())              //Man-mode a.k.a manual DSN definition
        m_testDb.setDatabaseName("Driver={"\
                               +m_dbContModel->index(m_cellIndex.row(),4).data().toString()+"};DATABASE="\
                               +m_dbContModel->index(m_cellIndex.row(),2).data().toString()+";");
    m_testDb.setUserName(m_dbContModel->index(m_cellIndex.row(),5).data().toString());
    m_testDb.setPassword(m_dbContModel->index(m_cellIndex.row(),6).data().toString());
    m_testDb.setHostName(m_dbContModel->index(m_cellIndex.row(),7).data().toString());
    m_testDb.setPort(m_dbContModel->index(m_cellIndex.row(),8).data().toInt());
    qDebug()<<m_testDb.databaseName();

    //Checkup
    if(m_testDb.open())
        m_dbContModel->setData(m_dbContModel->index(m_cellIndex.row(),9),"ALIVE");
    else
    {
        m_dbContModel->setData(m_dbContModel->index(m_cellIndex.row(),9),"DEAD");
        QMessageBox::warning(this,"DEAD","Connection failed with message:\n\n"+m_testDb.lastError().text());
    }
    m_testDb.close();
    m_testDb.removeDatabase("tests");
    this->refreshServers();
}

void MainWindow::checkAll()                                                             //Above function for all entries
{
    for(int i=0;i<m_dbContModel->rowCount();i++)
    {
        m_cellIndex = m_dbContModel->index(i,0);
        this->checkAction();
    }
}

void MainWindow::nextStep()                                                             //Open the next step, destroy this one.
{
    Step2 *secondStep = new Step2(NULL);
    secondStep->show();
    this->close();
}
