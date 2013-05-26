#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QtCore>
#include <QtGui>
#include <QtSql>

#include "enhancedsqltablemodel.h"
#include "changedbs.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    QSqlDatabase dbContainer;
    QSqlQuery *dbContQuery;

    EnhancedSqlTableModel *dbContModel;             //Sub-classed "just-in-case"
    Ui::MainWindow *ui;
    ChangeDBs *changedbs;
private slots:
    void editorClosed();
};

#endif // MAINWINDOW_H
