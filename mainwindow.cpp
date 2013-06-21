#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Center window on screen
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                          Qt::AlignCenter,
                                          this->size(),
                                          qApp->desktop()->availableGeometry()));
    this->ui->statusBar->setSizeGripEnabled(false);

    /*********************************************************************
     *** Open/create SQLite file for stored database connection options
     *********************************************************************/

    QSqlDatabase dbContainer = QSqlDatabase::addDatabase("QSQLITE");
    dbContainer.setDatabaseName("../DB-MigrationTool/dbCont.sqlite");                   //TODO : Change this path to reflect the release path
    dbContainer.open();
    m_dbContQuery = new QSqlQuery("CREATE TABLE IF NOT EXISTS conn(idx INTEGER PRIMARY KEY ASC, "
                                  "name TEXT, databaseName TEXT  driver TEXT, user TEXT, pass TEXT, "
                                  "hostname TEXT, port INTEGER, status TEXT)",dbContainer);

    //Setup the model
    m_dbContModel = new EnhancedSqlTableModel(this);
    m_dbContModel->setTable("conn");
    m_dbContModel->select();
    m_dbContModel->setHeaderData(1, Qt::Horizontal, "Name", Qt::DisplayRole);
    m_dbContModel->setHeaderData(2, Qt::Horizontal, "Database/DSN", Qt::DisplayRole);
    m_dbContModel->setHeaderData(7, Qt::Horizontal, "Host", Qt::DisplayRole);
    m_dbContModel->setHeaderData(8, Qt::Horizontal, "Port", Qt::DisplayRole);
    m_dbContModel->setHeaderData(9, Qt::Horizontal, "Status", Qt::DisplayRole);
    m_dbContModel->setEditStrategy(QSqlTableModel::OnFieldChange);\

    //"RESET" status column to "UNKNOWN"
    for (int i=0;i<m_dbContModel->rowCount(QModelIndex());i++)
        m_dbContModel->setData(m_dbContModel->index(i,9,QModelIndex()), "UNKNOWN", Qt::EditRole);

    //Setup the table
    ui->tabServers->setModel(m_dbContModel);
    ui->tabServers->hideColumn(0);                                                      //Hide, not remove, as not to ruin primary key.
    ui->tabServers->resizeColumnsToContents();
    ui->tabServers->resizeRowsToContents();
    ui->tabServers->setColumnWidth(1,240);
    ui->tabServers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);  //Stretch first column to window
    ui->tabServers->hideColumn(3);                                                      //Don't show user/pass and driver
    ui->tabServers->hideColumn(4);
    ui->tabServers->hideColumn(5);
    ui->tabServers->hideColumn(6);


    /**************
     *** Buttons
     **************/

    //Add
    connect(ui->btnAdd, SIGNAL(clicked()), SLOT(addAction()));

    //Edit
    connect(ui->btnEdit, SIGNAL(clicked()), SLOT(editAction()));

    //Remove
    connect(ui->btnRemove, SIGNAL(clicked()), SLOT(removeAction()));

    //Check Status
    connect(ui->btnStatus, SIGNAL(clicked()), SLOT(checkAll()));

    //Make Edit and Remove respond to table selection
    connect(ui->tabServers, SIGNAL(clicked(QModelIndex)), SLOT(enableButtons(QModelIndex)));
    connect(ui->tabServers, SIGNAL(selectionCleared()), SLOT(disableButtons()));

    //Menu
    ui->tabServers->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tabServers, SIGNAL(customContextMenuRequested(QPoint)), SLOT(serversContextMenu(QPoint)));

    //Next
    connect(ui->cmdNextStep, SIGNAL(clicked()), SLOT(nextStep()));
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
    if (index.isValid()) {
        m_cellIndex = ui->tabServers->currentIndex();                                   //Save index first
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
    QMenu *mnuServers = new QMenu();
    m_cellIndex = ui->tabServers->indexAt(pos);                                         //Save index first
    if (m_cellIndex.isValid()) {
        mnuServers->addAction("Add new...", this, SLOT(addAction()));
        mnuServers->addAction("Edit item...", this, SLOT(editAction()));
        mnuServers->addAction("Remove item",this,SLOT(removeAction()));
        mnuServers->addSeparator();
        mnuServers->addAction("Check connectivity",this,SLOT(checkAction()));
        mnuServers->exec(ui->tabServers->mapToGlobal(pos));                             //Map menu
    }
}

void MainWindow::addAction()                                                            //"Add" action for button and context-menu
{                                                                                       // ^- self explanatory
    m_indexToBeEdited = "0";
    ChangeDbs *adddbs = new ChangeDbs();
    if (adddbs->exec() != ChangeDbs::Accepted)
        return;

    m_dbContQuery->clear();
    m_dbContQuery->prepare("INSERT INTO conn (name, databaseName, driver, option, user, pass, "
                           "hostname, port) VALUES (?, ?, ?, ?, ?, ?, ?, ?);");
    m_dbContQuery->addBindValue(adddbs->name());
    m_dbContQuery->addBindValue(adddbs->dbName());
    m_dbContQuery->addBindValue(adddbs->driver());
    m_dbContQuery->addBindValue(adddbs->options());
    m_dbContQuery->addBindValue(adddbs->userName());
    m_dbContQuery->addBindValue(adddbs->password());
    m_dbContQuery->addBindValue(adddbs->hostName());
    m_dbContQuery->addBindValue(adddbs->port());

    m_dbContQuery->exec();
    this->refreshServers();
}

void MainWindow::editAction()                                                           //Same as "Add" but with prefilled form
{
    ChangeDbs *editdbs = new ChangeDbs(this);
    editdbs->setWindowTitle("Edit database...");
    //Populate
    m_indexToBeEdited = m_cellIndex.sibling(m_cellIndex.row(),0).data().toString();
    editdbs->setName(m_cellIndex.sibling(m_cellIndex.row(),1).data().toString());
    editdbs->setDbName(m_cellIndex.sibling(m_cellIndex.row(),2).data().toString());
    editdbs->setDriver(m_cellIndex.sibling(m_cellIndex.row(),3).data().toString());
    editdbs->setOptions(m_cellIndex.sibling(m_cellIndex.row(),4).data().toString());
    editdbs->setUserName(m_cellIndex.sibling(m_cellIndex.row(),5).data().toString());
    editdbs->setPassword(m_cellIndex.sibling(m_cellIndex.row(),6).data().toString());
    editdbs->setHostName(m_cellIndex.sibling(m_cellIndex.row(),7).data().toString());
    editdbs->setPort(m_cellIndex.sibling(m_cellIndex.row(),8).data().toString());
    if (!m_cellIndex.sibling(m_cellIndex.row(),4).data().toString().isEmpty())
        editdbs->overrideDSN(true);
    if (editdbs->exec() != ChangeDbs::Accepted)
        return;

    //Update where needed
    m_dbContQuery->clear();
    m_dbContQuery->prepare("UPDATE conn SET "
                           "name = ? , "
                           "databaseName = ? , "
                           "driver = ? , "
                           "option = ? , "
                           "user = ? , "
                           "pass = ? , "
                           "hostname = ? , "
                           "port = ? "
                           "WHERE idx = ?;");

    m_dbContQuery->addBindValue(editdbs->name());
    m_dbContQuery->addBindValue(editdbs->dbName());
    m_dbContQuery->addBindValue(editdbs->driver());
    m_dbContQuery->addBindValue(editdbs->options());
    m_dbContQuery->addBindValue(editdbs->userName());
    m_dbContQuery->addBindValue(editdbs->password());
    m_dbContQuery->addBindValue(editdbs->hostName());
    m_dbContQuery->addBindValue(editdbs->port());
    m_dbContQuery->addBindValue(m_indexToBeEdited);

    m_dbContQuery->exec();
    this->refreshServers();
}

void MainWindow::removeAction()                                                         //DROP
{
    this->m_dbContModel->removeRow(m_cellIndex.row());
    this->m_dbContModel->select();
}

void MainWindow::checkAction()                                                          //Check status of selected server
{
    int row = m_cellIndex.row();
    //Define a database with credentials from the table
    QSqlDatabase testDb = QSqlDatabase::addDatabase(m_dbContModel->index(row, 3).data().toString(),
                                                    "tests");
    testDb.setDatabaseName(m_dbContModel->index(row, 2).data().toString());
    if (!m_dbContModel->index(row, 4).data().toString().isEmpty())                      //Man-mode a.k.a manual DSN definition
        testDb.setDatabaseName(QString("Driver={%1};DATABASE=%2;").arg(m_dbContModel->index(row, 4).data().toString(),
                                                                       m_dbContModel->index(row, 2).data().toString()));
    testDb.setUserName(m_dbContModel->index(row,5).data().toString());
    testDb.setPassword(m_dbContModel->index(row,6).data().toString());
    testDb.setHostName(m_dbContModel->index(row,7).data().toString());
    testDb.setPort(m_dbContModel->index(row,8).data().toInt());

    //Checkup
    if(testDb.open())
        m_dbContModel->setData(m_dbContModel->index(row, 9),QStringLiteral("ALIVE"));
    else {
        m_dbContModel->setData(m_dbContModel->index(row, 9),QStringLiteral("DEAD"));
        QMessageBox::warning(this, "DEAD", QString("Connection failed with message:\n\n%1").arg(testDb.lastError().text()));
    }
    testDb.close();
    testDb.removeDatabase("tests");
    this->refreshServers();
}

void MainWindow::checkAll()                                                             //Above function for all entries
{
    for (int i=0;i<m_dbContModel->rowCount();i++) {
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
