#include "enhancedsqltablemodel.h"
#include <QColor>



EnhancedSqlTableModel::EnhancedSqlTableModel(QObject *parent) :
    QSqlTableModel(parent)
{   
}

//Set STATUS column as not editable
Qt::ItemFlags EnhancedSqlTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlTableModel::flags(index);
    if ((index.column() == 9)) flags &=~Qt::ItemIsEnabled;                  //Not editable by user, but still changeable
    return flags;
}
