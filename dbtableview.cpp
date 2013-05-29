#include "dbtableview.h"
#include <QMouseEvent>

DBTableView::DBTableView(QWidget *parent) :
    QTableView(parent)
{
}
// Enable deselection for table
void DBTableView::mousePressEvent(QMouseEvent *event)
{
   if (event->button()==Qt::LeftButton && !(this->indexAt(event->pos()).isValid()))
   {
       this->clearSelection();
       emit selectionCleared();
   }
   QTableView::mousePressEvent(event);
}
