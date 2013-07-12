#include "step2.h"
#include "ui_step2.h"
#include <QDebug>


Step2::Step2(QWidget *parent) :
    QMainWindow(parent), 
    ui(new Ui::Step2)
{
    ui->setupUi(this);

    //Center window on screen
    initUi();

    QGraphicsDropShadowEffect *dropshadow = new QGraphicsDropShadowEffect;
    dropshadow->setBlurRadius(40);
    dropshadow->setXOffset(1);
    dropshadow->setYOffset(1);
    dropshadow->setColor(Qt::black);

    // Open/create SQLite file for stored database connection options
    initSqlite();

    //Setup comboboxes
    ui->cmbFirstDB->setModel(m_dbtCont);
    ui->cmbSecondDB->setModel(m_dbtCont);

    ui->cmbFirstDB->setModelColumn(1);
    ui->cmbSecondDB->setModelColumn(1);

    ui->cmbFirstDB->setCurrentIndex(-1);
    ui->cmbSecondDB->setCurrentIndex(-1);

    //Remove offline databases
    connect(this->ui->cmbFirstDB, SIGNAL(currentIndexChanged(int)), SLOT(checkTable(int)));
    connect(this->ui->cmbSecondDB, SIGNAL(currentIndexChanged(int)), SLOT(checkTable(int)));

    //Context-menus
    connect(this->ui->tabFirstDB, SIGNAL(customContextMenuRequested(QPoint)), SLOT(dbContext(QPoint)));
    connect(this->ui->tabSecondDB, SIGNAL(customContextMenuRequested(QPoint)), SLOT(dbContext(QPoint)));

    //Back button
    connect(this->ui->cmdPrev, SIGNAL(clicked()), SLOT(closeAction()));

    //Compare tables button
    connect(this->ui->btnCompare, SIGNAL(clicked()), SLOT(compareTablesAction()));

    //Compare by content button
    connect(this->ui->btnCompareByHeaders, SIGNAL(clicked()), SLOT(compareByHeaderStep1Action()));

    //Reset comparison button
    connect(this->ui->btnReset, SIGNAL(clicked()), SLOT(resetCompare()));

    //Initial disabled state
    ui->btnReset->setVisible(false);
    ui->btnCompare->setEnabled(false);
    ui->btnCompareByHeaders->setEnabled(false);
}

Step2::~Step2()
{
    m_testDb.close();
    delete ui;
}

void Step2::initUi()
{
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                          Qt::AlignCenter,
                                          this->size(),
                                          qApp->desktop()->availableGeometry()));
    this->ui->statusBar->setSizeGripEnabled(false);
}

void Step2::initSqlite()
{
    QSqlDatabase dbContainer = QSqlDatabase::addDatabase("QSQLITE", "dbContainer");
    dbContainer.setDatabaseName("../DB-MigrationTool/dbCont.sqlite");                   //TODO : Change this path to reflect the release path
    dbContainer.open();

    //DB init
    m_dbtCont = new QSqlTableModel(NULL, dbContainer);
    m_dbtCont->setEditStrategy(QSqlTableModel::OnFieldChange);
    m_dbtCont->setTable("conn");
    m_dbtCont->setFilter("status != 'DEAD'");
    m_dbtCont->select();
}
void Step2::preCompare()
{
    //Keep tab on who has what
    m_countF = ui->tabFirstDB->model()->rowCount();
    m_countS = ui->tabSecondDB->model()->rowCount();

    for (int i = 0; i < m_countF; i++)
        m_listF << ui->tabFirstDB->model()->index(i, 0).data().toString();

    for (int i = 0; i < m_countS; i++)
        m_listS << ui->tabSecondDB->model()->index(i, 0).data().toString();

    //Keep them neat
    qSort(m_listF);
    qSort(m_listS);

    ui->tabFirstDB->sortByColumn(0, Qt::AscendingOrder);
    ui->tabSecondDB->sortByColumn(0, Qt::AscendingOrder);

    //Comparison
    for (int i = 0; i < m_countF; i++)
        for (int j = 0; j < m_countS; j++)
            if (m_listF.at(i) == m_listS.at(j))
                m_commonList << m_listF.at(i);

    //Remove common tables (for marking)
    foreach (QString str, m_commonList) {
        m_listF.removeAll(str);
        m_listS.removeAll(str);
    }
}

void Step2::postCompare()
{
    m_listF.clear();
    m_listS.clear();
    m_commonList.clear();
}

void Step2::refresh()
{
    int currentIndexF = ui->cmbFirstDB->currentIndex();
    int currentIndexS = ui->cmbSecondDB->currentIndex();

    ui->cmbFirstDB->setCurrentIndex(-1);
    ui->cmbSecondDB->setCurrentIndex(-1);

    ui->cmbFirstDB->setCurrentIndex(currentIndexF);
    ui->cmbSecondDB->setCurrentIndex(currentIndexS);
}

void Step2::openDbFromCredentials(int row, TabView *table)
{
    m_testDb = QSqlDatabase::addDatabase(m_dbtCont->index(row, 3).data().toString(),
                                         QString("tests-%1").arg(table->objectName()));
    m_testDb.setDatabaseName(m_dbtCont->index(row, 2).data().toString());
    if (!m_dbtCont->index(row, 4).data().toString().isEmpty())                          //Man-mode a.k.a manual DSN definition
        m_testDb.setDatabaseName(QString("Driver={%1};DATABASE=%2;").arg(m_dbtCont->index(row, 4).data().toString(),
                                                                         m_dbtCont->index(row, 2).data().toString()));
    m_testDb.setUserName(m_dbtCont->index(row, 5).data().toString());
    m_testDb.setPassword(m_dbtCont->index(row, 6).data().toString());
    m_testDb.setHostName(m_dbtCont->index(row, 7).data().toString());
    m_testDb.setPort(m_dbtCont->index(row, 8).data().toInt());
}

void Step2::setHeaderNote()
{
    if ((ui->cmbFirstDB->currentIndex() >= 0) && ui->cmbSecondDB->currentIndex() >= 0) {
        ui->lblHeader->setText("You can copy tables between databases by <strong>dragging and dropping</strong> <br />them from one database to the other.");
        ui->btnCompare->setEnabled(true);
        ui->btnCompareByHeaders->setEnabled(true);
    }
}

void Step2::keepTrackOfQueriesAndDbs()
{
    if (sender()->objectName() == QStringLiteral("cmbFirstDB"))
        m_dbF = m_testDb;
    if (sender()->objectName() == QStringLiteral("cmbSecondDB"))
        m_dbS = m_testDb;

    if (sender()->objectName() == QStringLiteral("cmbFirstDB"))
        m_dbQueryF = QSqlQuery(m_dbF);
    if (sender()->objectName() == QStringLiteral("cmbSecondDB"))
        m_dbQueryS = QSqlQuery(m_dbS);

    qDebug() << sender()->objectName();
}

void Step2::checkTableRedundancy(int row)
{
    if (row != m_oldRow)
        m_dbtCont->select();
    else {
        if(sender()->objectName() == QStringLiteral("cmbFirstDB"))
            ui->cmbFirstDB->setCurrentIndex(m_oldRow);
        if(sender()->objectName() == QStringLiteral("cmbSecondDB"))
            ui->cmbSecondDB->setCurrentIndex(m_oldRow);
    }
    if ((sender()->objectName() == QStringLiteral("cmbFirstDB")) && (row != m_oldRow)) {
        ui->cmbFirstDB->setCurrentIndex(row);
        row=-1;
    }
    if ((sender()->objectName() == QStringLiteral("cmbSecondDB")) && (row != m_oldRow)) {
        ui->cmbSecondDB->setCurrentIndex(row);
        row=-1;
    }
}

QStandardItemModel * Step2::setupModelForSender(TabView *table)
{
    QStandardItemModel *model = new QStandardItemModel(table);
    model->insertColumns(0, 1);
    model->insertRows(0, m_testDb.tables().count());
    model->setHeaderData(0, Qt::Horizontal, "Table name", Qt::DisplayRole);

    return model;
}

void Step2::populateModelForSender(TabView *table, QStandardItemModel *model)
{
    for (int i=0;i<m_testDb.tables().count();++i)
        model->setData(model->index(i, 0), m_testDb.tables().at(i));

    table->setModel(model);
    table->refresh();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void Step2::checkTable(int row)
{
    //Hack - for removal of offline DBs - keep record of the previous option
    if(row == -1)
        row=m_oldRow;

    //Change note on top.
    setHeaderNote();

    //Check the signal sender
    TabView *table = ui->tabFirstDB;
    if(sender()->objectName() == "cmbSecondDB")
        table = ui->tabSecondDB;

    //Open a database with credentials provided by SQLite DB
    openDbFromCredentials(row, table);

    //Make copies of DBs and queries for later use and easier identification
    keepTrackOfQueriesAndDbs();

    //Error? Mark as DEAD and return
    if (!m_testDb.open()) {
        m_dbtCont->setData(m_dbtCont->index(row, 9), "DEAD");
        QMessageBox::warning(this, "DEAD", QString("Connection failed with message:\n\n %1").arg(m_testDb.lastError().text()));
        m_dbtCont->setFilter(QStringLiteral("status != 'DEAD'"));
        m_dbtCont->select();
        return;
    } else {
        m_oldRow=row;
        m_dbtCont->setData(m_dbtCont->index(row, 9), "ALIVE");
    }

    //Redundancy check - we don't want a loop here.
    checkTableRedundancy(row);

    //Setup model for the calling table (sender())
    QStandardItemModel *model = setupModelForSender(table);

    // ...and populate
    populateModelForSender(table, model);

    //Drag-and-drop connection
    connect(table, SIGNAL(dbTableMoved(QString, QString)), SLOT(dbTableMove(QString, QString)));
}

void Step2::repopulateDestinationTable(QString table, QSqlDatabase destDb, QSqlQuery destQuery, QSqlQuery srcQuery)
{
    destDb.transaction();                                                           // Near-lightspeed transaction
    while (srcQuery.next()) {
        QSqlRecord record = srcQuery.record();
        QStringList names;
        QStringList values;

        for (int i = 0; i < record.count(); ++i) {
            names << record.fieldName(i);
            values << QString("\"%1\"").arg(record.value(i).toString());
        }

        //Build new query
        QString queryStr;
        queryStr.append(QString("INSERT INTO %1 (%2) VALUES (%3)").arg(table, names.join(", "), values.join(", ")));
        destQuery.exec(queryStr);
    }
    destDb.commit();
}

QString Step2::setupDestinationTable(QString table, QSqlQuery srcQuery)
{
    QString tableCreateStr = QString("CREATE TABLE %1 (").arg(table);
    QString pri = "(";                                                                  // pri: same concat, for PRI KEYS

    while (srcQuery.next()) {
        tableCreateStr.append(QString(" %1 %2, ").arg(srcQuery.record().value(0).toString(),
                                                     srcQuery.record().value(1).toString()));
        if (srcQuery.record().value(2).toString() == QStringLiteral("PRI"))
            pri.append(QString("%1, ").arg(srcQuery.record().value(0).toString()));
    }
    pri.resize(pri.size()-2);
    pri.append(")");
    tableCreateStr.append(QString(" PRIMARY KEY %1)").arg(pri));

    return tableCreateStr;
}

void Step2::dbTableMove(QString table, QString source)
{
    //Who's calling this?
    QSqlQuery srcQuery;
    QSqlQuery destQuery;
    QSqlDatabase destDb;                                                                //For using transaction and commit

    if (source == SECOND) {
        srcQuery = QSqlQuery(m_dbQueryS);
        destQuery = QSqlQuery(m_dbQueryF);
        destDb = QSqlDatabase(m_dbF);
    }

    if (source == FIRST) {
        srcQuery = QSqlQuery(m_dbQueryF);
        destQuery = QSqlQuery(m_dbQueryS);
        destDb = QSqlDatabase(m_dbS);
    }

    destQuery = QSqlQuery(destDb);
    // Get table data via INFO SCHEMA - compatible with MySQL and SQL Server (and more maybe)
    //  !!!!!!!!!!! TODO: ADD AUTO-INCREMENT AND ISNULL
    if (!srcQuery.exec(QString("SELECT COLUMN_NAME, COLUMN_TYPE, "
                               "COLUMN_KEY FROM INFORMATION_SCHEMA.COLUMNS "
                               "WHERE TABLE_NAME = '%1'").arg(table)))
        return;

    // Concatenate a query string for recreating tables on the destination server
    QString tableCreateStr = setupDestinationTable(table, srcQuery);

    //DROP destination if exists
    if (!destQuery.exec(QString("DROP TABLE IF EXISTS %1").arg(table)))
        return;

    //"Create" query exec
    if (!destQuery.exec(tableCreateStr))
        return;

    //Repopulate transferred table if we may copy contents
    if (!ui->chkCopyWithoutContents->isChecked()) {
        if (!srcQuery.exec(QString("SELECT * FROM %1").arg(table)))
            return;
        repopulateDestinationTable(table, destDb, destQuery, srcQuery);
    }
}

void Step2::dbContext(const QPoint &pos)                                                //Context-menu definition for tables
{
    //Again, who's calling?
    TabView *table = ui->tabFirstDB;
    m_dbContextSource = FIRST;
    if (sender()->objectName() == QStringLiteral("tabSecondDB")) {
        table = ui->tabSecondDB;
        m_dbContextSource = SECOND;
    }

    //Get an index for the right-clicked position
    m_cellIndex = table->indexAt(pos);
    QMenu *mnuDb = new QMenu();

    if (m_cellIndex.isValid()) {
        mnuDb->addAction("Delete table", this, SLOT(removeTableAction()));              //The context-menu
        mnuDb->addSeparator();
        mnuDb->addAction("View table contents", this, SLOT(viewTableAction()));
        mnuDb->exec(table->mapToGlobal(pos));
    }
}

void Step2::removeTableAction()                                                         //Drop table funct.
{
    if (QMessageBox::warning(this, "Remove (DROP) table from database", \
                             "Are you sure you want to DROP the selected table from the database?", \
                             QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel)
        return;

    if (m_dbContextSource == FIRST) {
        m_dbQueryF.exec(QString("DROP TABLE %1").arg(m_cellIndex.sibling(m_cellIndex.row(), 0).data().toString()));
        ui->tabFirstDB->model()->removeRow(m_cellIndex.row());
        ui->tabFirstDB->refresh();
        ui->cmbFirstDB->setEnabled(false);                                              //BUG - Disabled this or eat segmentation fault
    } else if(m_dbContextSource == SECOND) {
        m_dbQueryS.exec(QString("DROP TABLE %1").arg(m_cellIndex.sibling(m_cellIndex.row(), 0).data().toString()));
        ui->tabSecondDB->model()->removeRow(m_cellIndex.row());
        ui->tabSecondDB->refresh();
        ui->cmbSecondDB->setEnabled(false);                                             //BUG - Disabled this or eat segmentation fault
    }
}

QPushButton * Step2::setupViewWindow(QTableView *table, QWidget *viewTable)
{
    QLabel *label = new QLabel("Table is editable. Press <b>F2</b> or <b>double-click</b> cell to edit.");
    QPushButton *btnOk = new QPushButton("&Close");

    QHBoxLayout *upperwrap = new QHBoxLayout();
    QVBoxLayout *center = new QVBoxLayout();
    viewTable->setLayout(center);
    upperwrap->addWidget(btnOk);
    upperwrap->addWidget(label);
    upperwrap->addStretch();
    center->addLayout(upperwrap);
    center->addWidget(table);

    return btnOk;
}

void Step2::viewTableAction()                                                           //Table quick-view
{
    //Custom "handwritten" window
    QWidget *viewTable = new QWidget(this);
    viewTable->setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint);
    viewTable->setGeometry(QCursor::pos().x(), QCursor::pos().y(), 400, 300);

    QTableView *table = new QTableView();
    QPushButton *btnOk = setupViewWindow(table, viewTable);
    QSqlTableModel *model = new QSqlTableModel(this, m_dbF);

    //Wrong model? Oops. Sorry, will redefine. (There's no other way to change the db it's connecting to)
    if (m_dbContextSource == SECOND) {
        delete model;
        QSqlTableModel *model = new QSqlTableModel(this, m_dbS);
    }
    model->setTable(m_cellIndex.sibling(m_cellIndex.row(), 0).data().toString());
    model->setEditStrategy(QSqlTableModel::OnFieldChange);                              //Nice and editable
    model->select();
    table->setModel(model);
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    connect(btnOk, SIGNAL(clicked()), viewTable, SLOT(close()));                        //Destroy button
    viewTable->show();
}

void Step2::markNonEquiv()
{
    for (int i = 0; i < m_countF; i++)
        if (m_listF.contains(ui->tabFirstDB->model()->index(i, 0).data().toString())) {
            ui->tabFirstDB->model()->setData(ui->tabFirstDB->model()->index(i, 0), qVariantFromValue(QFont(QFont().defaultFamily(), -1, 75)), Qt::FontRole);
            ui->tabFirstDB->model()->setData(ui->tabFirstDB->model()->index(i, 0), qVariantFromValue(QColor(Qt::red)), Qt::TextColorRole);
        }

    for (int i = 0; i < m_countS; i++)
        if (m_listS.contains(ui->tabSecondDB->model()->index(i, 0).data().toString())) {
            ui->tabSecondDB->model()->setData(ui->tabSecondDB->model()->index(i, 0), qVariantFromValue(QFont(QFont().defaultFamily(), -1, 75)), Qt::FontRole);
            ui->tabSecondDB->model()->setData(ui->tabSecondDB->model()->index(i, 0), qVariantFromValue(QColor(Qt::red)), Qt::TextColorRole);
        }

    ui->lblHeader->setText(ui->lblHeader->text().append(" <i><strong>Non-equivalent tables have been marked in <span style=\"color:#ff0000\">red</span></strong></i>"));
}

void Step2::compareTablesAction()
{
    preCompare();

    //Marking
    markNonEquiv();

    //Allow reset
    ui->btnReset->setVisible(true);

    //Clear lists
    postCompare();
}

void Step2::removeNonEquiv()
{
    for (int i = m_countF; i >= 0; --i)
        if (m_listF.contains(ui->tabFirstDB->model()->index(i, 0).data().toString()))
            ui->tabFirstDB->model()->removeRow(i);

    for (int i = m_countS; i >= 0; --i)
        if (m_listS.contains(ui->tabSecondDB->model()->index(i, 0).data().toString()))
            ui->tabSecondDB->model()->removeRow(i);
}

void Step2::compareByHeaderStep1Action()
{
    preCompare();
    removeNonEquiv();

    ui->btnCompare->setEnabled(false);

    ui->btnCompareByHeaders->disconnect();
    ui->btnCompareByHeaders->setEnabled(false);

    ui->lblHeader->setText("<i><strong>Select one table from one of the databases and click \"Compare by content\" again.</strong></i>");
    ui->btnReset->setVisible(true);

    connect(this->ui->tabFirstDB, SIGNAL(clicked(QModelIndex)), SLOT(doubleSelect(QModelIndex)));
    connect(this->ui->tabSecondDB, SIGNAL(clicked(QModelIndex)), SLOT(doubleSelect(QModelIndex)));
    postCompare();
}

void Step2::compareByHeaderStep2Action()
{
    QSqlQuery queryF = QSqlQuery(m_dbQueryF);
    QSqlQuery queryS = QSqlQuery(m_dbQueryS);

    if (!queryF.exec(QString("SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS "
                             "WHERE TABLE_NAME = '%1'").arg(ui->tabFirstDB->currentIndex().data().toString()))) {
        qDebug() << "queryF not available.";
        return;
    }

    if (!queryS.exec(QString("SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS "
                             "WHERE TABLE_NAME = '%1'").arg(ui->tabSecondDB->currentIndex().data().toString()))) {
        qDebug() << "queryS not available.";
        return;
    }

    QStringList headerF;
    QStringList headerS;

    while(queryF.next())
        headerF.append(queryF.value(0).toString());

    while(queryS.next())
        headerS.append(queryS.value(0).toString());

    headerF.removeDuplicates();
    headerS.removeDuplicates();

    qDebug() << headerF.join("::");
    qDebug() << headerS.join("::");

    if(!(headerF == headerS) || (headerF.count() != headerS.count())) {
        QMessageBox::critical(this, "Tables not equivalent", "The tables have the same name but different headers.\n\nComparison aborted.");
        resetCompare();
        return;
    }

    ui->btnCompareByHeaders->disconnect();
    connect(this->ui->btnCompareByHeaders, SIGNAL(clicked()), SLOT(compareByHeaderStep1Action()));
}

void Step2::doubleSelect(QModelIndex index)
{
    if(sender()->objectName() == QStringLiteral("tabFirstDB"))
        ui->tabSecondDB->selectRow(index.row());
    if(sender()->objectName() == QStringLiteral("tabSecondDB"))
        ui->tabFirstDB->selectRow(index.row());

    ui->btnCompareByHeaders->setEnabled(true);
    connect(this->ui->btnCompareByHeaders, SIGNAL(clicked()), SLOT(compareByHeaderStep2Action()));
}

void Step2::resetCompare()
{
    refresh();

    ui->btnReset->setVisible(false);

    ui->btnCompareByHeaders->disconnect();
    ui->tabFirstDB->disconnect(SIGNAL(clicked(QModelIndex)));
    ui->tabSecondDB->disconnect(SIGNAL(clicked(QModelIndex)));
    connect(this->ui->btnCompareByHeaders, SIGNAL(clicked()), SLOT(compareByHeaderStep1Action()));

    ui->lblHeader->setText("You can copy tables between databases by <strong>dragging and dropping</strong> <br />them from one database to the other.");
}



void Step2::closeAction()
{
    MainWindow *firstStep = new MainWindow(NULL);                                       //Recreate first step window
    firstStep->show();
    this->close();
}
