#ifndef STEP2_H
#define STEP2_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QMenu>
#include <QWidget>
#include <QLayout>

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

private slots:
    void checkTable(int row);
    void dbTableMove(QString table, QString source);
    void dbContext(const QPoint &pos);
    void removeTableAction();
    void viewTableAction();

    void compareTablesAction();
    void closeAction();


signals:
//    void clicked();

};

#endif // STEP2_H
