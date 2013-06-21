#ifndef TABVIEW_H
#define TABVIEW_H

#include <QTableView>
#include <QApplication>
#include <QHeaderView>
#include <QtCore>
#include <QtGui>

#include <QDebug>

#define FIRST "F"
#define SECOND "S"

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

    QPoint m_dragStartPosition;
};

#endif // TABVIEW_H
