#ifndef DBTABLEVIEW_H
#define DBTABLEVIEW_H

#include <QTableView>

class DBTableView : public QTableView
{
    Q_OBJECT
public:
    explicit DBTableView(QWidget *parent = 0);
private:
    void mousePressEvent(QMouseEvent *event);
    
signals:
    void selectionCleared();
    
public slots:
    
};

#endif // DBTABLEVIEW_H
