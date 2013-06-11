#ifndef STEP2_H
#define STEP2_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QTableView>
#include <QMenu>
#include <QWidget>
#include <QLayout>

#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "enhancedsqltablemodel.h"
#include "mainwindow.h"

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
    QString dbContextSource;
    QModelIndex cellIndex;
    Ui::Step2 *ui;
    QSqlDatabase dbContainer, testDb, dbF, dbS;
    QSqlQuery dbQueryF, dbQueryS;
    QSqlTableModel *dbtCont;

    int oldRow;

private slots:
    void checkTable(int row);
    void dbTableMove(QString table, QString source);

    void dbContext(const QPoint &pos);

    void removeTableAction();
    void viewTableAction();

    void closeAction();

signals:
    void clicked();

};

/**********************************************************************************
 ***TABVIEW************************************************************************
 **********************************************************************************/

class TabView : public QTableView
{
    Q_OBJECT

public:
    explicit TabView(QWidget *parent = 0);
    ~TabView();

    void refresh();
signals:
    void dbTableMoved(QString table, QString dest);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dropEvent(QDropEvent *de);
    void dragMoveEvent(QDragMoveEvent *de);
    void dragEnterEvent(QDragEnterEvent *event);

    QPoint dragStartPosition;
};

#endif // STEP2_H
