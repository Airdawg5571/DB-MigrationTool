#ifndef STEP2_H
#define STEP2_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QMenu>
#include <QWidget>
#include <QLayout>
#include <QGraphicsDropShadowEffect>

#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "enhancedsqltablemodel.h"
#include "mainwindow.h"
#include "tabview.h"


namespace Ui {
class Step2;
}

class Step2 : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Step2(QWidget *parent = 0);
    ~Step2();


private:
    Ui::Step2 *ui;

    QString m_dbContextSource;
    QModelIndex m_cellIndex;
    QSqlDatabase m_testDb;
    QSqlDatabase m_dbF;
    QSqlDatabase m_dbS;
    QSqlQuery m_dbQueryF;
    QSqlQuery m_dbQueryS;
    QSqlTableModel *m_dbtCont;
    int m_oldRow;

    int m_countF;
    int m_countS;

    QStringList m_listF;
    QStringList m_listS;
    QStringList m_commonList;

    void preCompare();
    void postCompare();
    void refresh();

    void initUi();
    void initSqlite();
    void openDbFromCredentials(int row, TabView *table);
    void setHeaderNote();
    void keepTrackOfQueriesAndDbs();
    void checkTableRedundancy(int row);
    QStandardItemModel * setupModelForSender(TabView *table);
    void populateModelForSender(TabView *table, QStandardItemModel *model);
    void repopulateDestinationTable(QString table, QSqlDatabase destDb, QSqlQuery destQuery, QSqlQuery srcQuery);
    QString setupDestinationTable(QString table, QSqlQuery srcQuery);
    QPushButton * setupViewWindow(QTableView *table, QWidget *viewTable);
    void markNonEquiv();
    void removeNonEquiv();

private slots:
    void checkTable(int row);
    void dbTableMove(QString table, QString source);
    void dbContext(const QPoint &pos);
    void removeTableAction();
    void viewTableAction();
    void compareTablesAction();
    void compareByHeaderStep1Action();
    void compareByHeaderStep2Action();
    void doubleSelect(QModelIndex index);
    void resetCompare();
    void closeAction();

signals:
//    void clicked();

};

#endif // STEP2_H
