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
    model->insertColumns(0,1);
    model->insertRows(0,testDbF.tables().count());
    model->setHeaderData(0,Qt::Horizontal,"Table name", Qt::DisplayRole);

    for(int i=0;i<testDbF.tables().count();++i)
    {
        model->setData(model->index(i,0),testDbF.tables().at(i));
    }
    ui->tabFirstDB->setModel(model);
    ui->tabFirstDB->refresh();
    ui->tabFirstDB->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->tabFirstDB,SIGNAL(pressed(QModelIndex)),ui->tabFirstDB,SLOT(makeDrag(QModelIndex)));
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
    model->insertColumns(0,1);
    model->insertRows(0,testDbS.tables().count());
    model->setHeaderData(0,Qt::Horizontal,"Table name", Qt::DisplayRole);

    for(int i=0;i<testDbS.tables().count();++i)
    {
        model->setData(model->index(i,0),testDbS.tables().at(i));
    }
    ui->tabSecondDB->setModel(model);
    ui->tabSecondDB->refresh();
    ui->tabSecondDB->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->tabSecondDB,SIGNAL(pressed(QModelIndex)),ui->tabSecondDB,SLOT(makeDrag(QModelIndex)));
}
/**********************************************************
 **********************************************************
 **********************************************************/

TabView::TabView(QWidget *parent) :
    QTableView(parent)
{
}

TabView::~TabView()
{
}

void TabView::refresh()
{
    this->resizeColumnsToContents();
    this->resizeRowsToContents();
    this->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
}

void TabView::makeDrag(QModelIndex index)
{
    QDrag *dr = new QDrag(this);
    // The data to be transferred by the drag and drop operation is contained in a QMimeData object
    QMimeData *data = new QMimeData;
    if (this->objectName() == "tabFirstDB")
        data->setData("mime/first-text",index.data().toByteArray());
    if (this->objectName() == "tabSecondDB")
        data->setData("mime/second-text",index.data().toByteArray());
//    data->setText("This is a test");
    dr->setMimeData(data);
    dr->start();
}

void TabView::dropEvent(QDropEvent *de)
{
    // Unpack dropped data and handle it the way you want
    QModelIndex index = this->indexAt(de->pos());
    if(index.isValid())
    {
        this->model()->insertRow(index.row());
        if(de->mimeData()->hasFormat("mime/second-text")) this->model()->setData(index,de->mimeData()->data("mime/second-text"));
        if(de->mimeData()->hasFormat("mime/first-text")) this->model()->setData(index,de->mimeData()->data("mime/first-text"));
        this->refresh();
    }
    else
    {
        this->model()->insertRow(this->model()->rowCount());
        if(de->mimeData()->hasFormat("mime/second-text"))
            this->model()->setData(this->model()->index(this->model()->rowCount()-1,0),\
                                   de->mimeData()->data("mime/second-text"));
        if(de->mimeData()->hasFormat("mime/first-text"))
            this->model()->setData(this->model()->index(this->model()->rowCount()-1,0),\
                                   de->mimeData()->data("mime/first-text"));
        this->refresh();
    }

}

void TabView::dragMoveEvent(QDragMoveEvent *de)
{
    // The event needs to be accepted here
    de->accept();
}

void TabView::dragEnterEvent(QDragEnterEvent *event)
{
    // Set the drop action to be the proposed action.
    if (event->mimeData()->hasFormat("mime/first-text") && this->objectName() == "tabSecondDB")
             event->acceptProposedAction();
    if (event->mimeData()->hasFormat("mime/second-text") && this->objectName() == "tabFirstDB")
             event->acceptProposedAction();
}
