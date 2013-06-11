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

    // Open/create SQLite file for stored database connection options
    dbContainer = QSqlDatabase::addDatabase("QSQLITE", "dbContainer");
    dbContainer.setDatabaseName("../DB-MigrationTool/dbCont.sqlite");                   //TODO : Change this path to reflect the release path
    dbContainer.open();

    //DB init
    dbtCont = new QSqlTableModel(NULL,dbContainer);
    dbtCont->setEditStrategy(QSqlTableModel::OnFieldChange);
    dbtCont->setTable("conn");
    dbtCont->setFilter("status != 'DEAD'");
    dbtCont->select();

    //Setup comboboxes
    ui->cmbFirstDB->setModel(dbtCont);
    ui->cmbSecondDB->setModel(dbtCont);

    ui->cmbFirstDB->setModelColumn(1);
    ui->cmbSecondDB->setModelColumn(1);

    ui->cmbFirstDB->setCurrentIndex(-1);
    ui->cmbSecondDB->setCurrentIndex(-1);

    //Remove offline databases
    connect(this->ui->cmbFirstDB,SIGNAL(currentIndexChanged(int)),SLOT(checkTable(int)));
    connect(this->ui->cmbSecondDB,SIGNAL(currentIndexChanged(int)),SLOT(checkTable(int)));

    //Context-menus
    connect(this->ui->tabFirstDB,SIGNAL(customContextMenuRequested(QPoint)),SLOT(dbContext(QPoint)));
    connect(this->ui->tabSecondDB,SIGNAL(customContextMenuRequested(QPoint)),SLOT(dbContext(QPoint)));

    //Back button
    connect(this->ui->cmdPrev,SIGNAL(clicked()),SLOT(closeAction()));

}

Step2::~Step2()
{
    testDb.close();
    delete ui;
}

void Step2::checkTable(int row)
{
    //Hack - for removal of offline DBs - keep record of the previous option

    if(row == -1)
    {
        row=oldRow;
    }

    //Check the signal sender
    TabView *table = ui->tabFirstDB;
    if(sender()->objectName() == "cmbSecondDB") table = ui->tabSecondDB;

    //Open a database with credentials provided by SQLite DB
    testDb = QSqlDatabase::addDatabase(dbtCont->index(row,3).data().toString(),"tests-"+table->objectName());
    testDb.setDatabaseName(dbtCont->index(row,2).data().toString());
    if(!dbtCont->index(row,4).data().toString().isEmpty())                              //Man-mode a.k.a manual DSN definition
        testDb.setDatabaseName("Driver={"\
                               +dbtCont->index(row,4).data().toString()+"};DATABASE="\
                               +dbtCont->index(row,2).data().toString()+";");
    testDb.setUserName(dbtCont->index(row,5).data().toString());
    testDb.setPassword(dbtCont->index(row,6).data().toString());
    testDb.setHostName(dbtCont->index(row,7).data().toString());
    testDb.setPort(dbtCont->index(row,8).data().toInt());

    //Make copies of DBs and queries for later use and easier identification
    if (sender()->objectName() == "cmbFirstDB")dbF = testDb;
    if (sender()->objectName() == "cmbSecondDB")dbS = testDb;

    if (sender()->objectName() == "cmbFirstDB") dbQueryF = QSqlQuery(dbF);
    if (sender()->objectName() == "cmbSecondDB") dbQueryS = QSqlQuery(dbS);

    //Error? Mark as DEAD and return
    if(!testDb.open())
    {
        dbtCont->setData(dbtCont->index(row,9),"DEAD");
        QMessageBox::warning(this,"DEAD","Connection failed with message:\n\n"+testDb.lastError().text());
        dbtCont->setFilter("status != 'DEAD'");
        dbtCont->select();
        return;
    }
    //(Not neccesary) ..but useful. Mark ALIVE connections
    else
    {
        oldRow=row;
        dbtCont->setData(dbtCont->index(row,9),"ALIVE");
    }

    //Redundancy check - we don't want a loop here.
    if (row != oldRow) dbtCont->select(); else
    {
        if(sender()->objectName() == "cmbFirstDB") ui->cmbFirstDB->setCurrentIndex(oldRow);
        if(sender()->objectName() == "cmbSecondDB") ui->cmbSecondDB->setCurrentIndex(oldRow);
    }
    if (sender()->objectName() == "cmbFirstDB" && row != oldRow) {ui->cmbFirstDB->setCurrentIndex(row); row=-1;}
    if (sender()->objectName() == "cmbSecondDB" && row != oldRow) {ui->cmbSecondDB->setCurrentIndex(row); row=-1;}

    //Setup model for the calling table (sender())
    QStandardItemModel *model = new QStandardItemModel(table);
    model->insertColumns(0,1);
    model->insertRows(0,testDb.tables().count());
    model->setHeaderData(0,Qt::Horizontal,"Table name", Qt::DisplayRole);

    // ...and populate
    for(int i=0;i<testDb.tables().count();++i)
    {
        model->setData(model->index(i,0),testDb.tables().at(i));
    }

    table->setModel(model);
    table->refresh();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //Drag-and-drop connection
    connect(table,SIGNAL(dbTableMoved(QString,QString)),SLOT(dbTableMove(QString,QString)));
}

void Step2::dbTableMove(QString table, QString source)
{

    //Who's calling this?
    QSqlQuery srcQuery = QSqlQuery(dbQueryS);
    QSqlQuery destQuery = QSqlQuery(dbQueryF);
    if(source == "F")
    {
        srcQuery = QSqlQuery(dbQueryF);
        destQuery = QSqlQuery(dbQueryS);
    }

    // Get table data via INFO SCHEMA - compatible with MySQL and SQL Server (and more maybe)
    //  !!!!!!!!!!! TODO: ADD AUTO-INCREMENT AND ISNULL
    if (!srcQuery.exec("SELECT COLUMN_NAME, COLUMN_TYPE, COLUMN_KEY FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME = '"+table+"'"))
    {
        qDebug()<<"[src]"+srcQuery.lastError().text();
        return;
    }

    // Concatenate a query string for recreating tables on the destination server
    QString tableCreateStr = "CREATE TABLE "+table+" (";
    QString pri = "(";                                                                  // pri: same concat, for PRI KEYS

    while(srcQuery.next())
    {
        tableCreateStr.append(" "+srcQuery.record().value(0).toString());
        tableCreateStr.append(" "+srcQuery.record().value(1).toString());
        if(srcQuery.record().value(2).toString() == "PRI") pri.append(srcQuery.record().value(0).toString()+", ");
        tableCreateStr.append(", ");
    }
    pri.resize(pri.size()-2);
    pri.append(")");
    tableCreateStr.append(" PRIMARY KEY "+pri);
    tableCreateStr.append(")");

    qDebug()<<"[table]"+tableCreateStr;

    //DROP destination if exists
    if (!destQuery.exec(QString("DROP TABLE IF EXISTS %1").arg(table)))
    {
        qDebug()<<"[des]"+destQuery.lastError().text();
        return;
    }

    //"Create" query exec
    if (!destQuery.exec(tableCreateStr))
    {
        qDebug()<<"[des2]"+destQuery.lastError().text();
        return;
    }
}

void Step2::dbContext(const QPoint &pos)                                                //Context-menu definition for tables
{
    //Again, who's calling?
    TabView *table = ui->tabFirstDB;
    dbContextSource = "F";
    if(sender()->objectName() == "tabSecondDB"){table = ui->tabSecondDB; dbContextSource = "S";}

    //Get an index for the right-clicked position
    cellIndex = table->indexAt(pos);
    QMenu *mnuDb = new QMenu();

    if(cellIndex.isValid())
    {
        mnuDb->addAction("Delete table",this,SLOT(removeTableAction()));                //The context-menu
        mnuDb->addSeparator();
        mnuDb->addAction("View table contents", this, SLOT(viewTableAction()));
        mnuDb->exec(table->mapToGlobal(pos));
    }
}

void Step2::removeTableAction()                                                         //Drop table funct.
{
    if(QMessageBox::warning(this,"Remove (DROP) table from database",\
                            "Are you sure you want to DROP the selected table from the database?",\
                            QMessageBox::Ok,QMessageBox::Cancel) == QMessageBox::Rejected)
        return;

    if(dbContextSource == "F")
    {
        dbQueryF.exec("DROP TABLE "+cellIndex.sibling(cellIndex.row(),0).data().toString());
        ui->tabFirstDB->model()->removeRow(cellIndex.row());
        ui->tabFirstDB->refresh();
        ui->cmbFirstDB->setEnabled(false);                                              //BUG - Disabled this or eat segmentation fault
    }

    if(dbContextSource == "S")
    {
        dbQueryS.exec("DROP TABLE "+cellIndex.sibling(cellIndex.row(),0).data().toString());
        ui->tabSecondDB->model()->removeRow(cellIndex.row());
        ui->tabSecondDB->refresh();
        ui->cmbSecondDB->setEnabled(false);                                             //BUG - Disabled this or eat segmentation fault
    }
}

void Step2::viewTableAction()                                                           //Table quick-view
{
    //Custom "handwritten" window
    QWidget *viewTable = new QWidget(this);
    viewTable->setWindowFlags(Qt::Tool|Qt::CustomizeWindowHint);
    viewTable->setGeometry(QCursor::pos().x(),QCursor::pos().y(),400,300);
    QTableView *table = new QTableView;
    QLabel *label = new QLabel("Table is editable. Press <b>F2</b> or <b>double-click</b> cell to edit.");
    QPushButton *btnOk = new QPushButton("&OK");

    QHBoxLayout *lowerwrap = new QHBoxLayout();
    QVBoxLayout *center = new QVBoxLayout();
    viewTable->setLayout(center);
    center->addWidget(label);
    center->addWidget(table);
    center->addLayout(lowerwrap);
    lowerwrap->addStretch();
    lowerwrap->addWidget(btnOk);
    QSqlTableModel *model = new QSqlTableModel(this,dbF);

    //Wrong model? Oops. Sorry, will redefine. (There's no other way to change the db it's connecting to)
    if(dbContextSource == "S") {delete model; QSqlTableModel *model = new QSqlTableModel(this,dbS);}
    model->setTable(cellIndex.sibling(cellIndex.row(),0).data().toString());
    model->setEditStrategy(QSqlTableModel::OnFieldChange);                              //Nice and editable
    model->select();
    table->setModel(model);
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    connect(btnOk,SIGNAL(clicked()),viewTable,SLOT(close()));                           //Destroy button
    viewTable->show();
}

void Step2::closeAction()
{
    MainWindow *firstStep = new MainWindow(NULL);                                       //Recreate first step window
    firstStep->show();
    this->close();
}

/**********************************************************
 ***TABVIEW************************************************
 **********************************************************/

//I know. Uncommon to write a class inside another class' files

TabView::TabView(QWidget *parent) :
    QTableView(parent)
{
    //Standard QTableView constructor and destructor. Nothing to write here.
}

TabView::~TabView()
{
}

void TabView::refresh()                                                                 //Keep things nice and tidy
{
    this->resizeColumnsToContents();
    this->resizeRowsToContents();
    this->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
}

void TabView::mouseMoveEvent(QMouseEvent *event)                                        //Drag-drop functionality here
{
    QModelIndex index = this->indexAt(event->pos());
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;
    QDrag *dr = new QDrag(this);
    // The data to be transferred by the drag and drop operation is contained in a QMimeData object
    QMimeData *data = new QMimeData;
    if (this->objectName() == "tabFirstDB")
        data->setData("text/first-text",index.data().toByteArray());
    if (this->objectName() == "tabSecondDB")
        data->setData("text/second-text",index.data().toByteArray());
    dr->setMimeData(data);
    dr->start();

}

void TabView::mousePressEvent(QMouseEvent *event)                                       //Get mousepos at click before resuming normal operation
{
    if (event->buttons() == Qt::LeftButton)
    {
        dragStartPosition = event->pos();
    }
    QTableView::mousePressEvent(event);
}

void TabView::dropEvent(QDropEvent *de)
{
    // Unpack dropped data and handle it
    QModelIndex index = this->indexAt(de->pos());
    if(index.isValid())
    {
        this->model()->insertRow(index.row());
        if(de->mimeData()->hasFormat("text/second-text")) this->model()->setData(index,de->mimeData()->data("text/second-text"));
        if(de->mimeData()->hasFormat("text/first-text")) this->model()->setData(index,de->mimeData()->data("text/first-text"));
        this->refresh();
    }
    else
    {
        this->model()->insertRow(this->model()->rowCount());
        if(de->mimeData()->hasFormat("text/second-text"))
        {
            this->model()->setData(this->model()->index(this->model()->rowCount()-1,0),\
                                   de->mimeData()->data("text/second-text"));
            emit dbTableMoved(de->mimeData()->data("text/second-text"),"S");
        }
        if(de->mimeData()->hasFormat("text/first-text"))
        {
            this->model()->setData(this->model()->index(this->model()->rowCount()-1,0),\
                                   de->mimeData()->data("text/first-text"));
            emit dbTableMoved(de->mimeData()->data("text/first-text"),"F");
        }
        this->refresh();
    }
}

void TabView::dragMoveEvent(QDragMoveEvent *de)
{
    // The event needs to be accepted here.
    de->accept();
}

void TabView::dragEnterEvent(QDragEnterEvent *event)
{
    // Set the drop action to be the proposed action. Again, to know who calls the drag.
    if (event->mimeData()->hasFormat("text/first-text") && this->objectName() == "tabSecondDB")
        event->acceptProposedAction();
    if (event->mimeData()->hasFormat("text/second-text") && this->objectName() == "tabFirstDB")
        event->acceptProposedAction();
}
