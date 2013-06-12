#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QMenu>

#include "enhancedsqltablemodel.h"
#include "changedbs.h"
#include "step2.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void refreshServers();
    
private:
    QSqlDatabase m_dbContainer, m_testDb;
    EnhancedSqlTableModel *m_dbContModel;
    Ui::MainWindow *ui;
    ChangeDBs *m_editdbs, *m_adddbs;
    QMenu *m_mnuServers;
    QModelIndex m_cellIndex;
    QString m_indexToBeEdited;

private slots:
    void enableButtons(QModelIndex index);
    void disableButtons();
    void serversContextMenu(const QPoint &pos);

    void addAction();
    void editAction();
    void removeAction();
    void checkAction();
    void checkAll();
    void nextStep();
};

#endif // MAINWINDOW_H
