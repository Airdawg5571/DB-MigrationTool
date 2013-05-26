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
    if ((index.column() == 5)) flags &=~Qt::ItemIsEnabled;                  //Not editable by user, but still changeable
    return flags;
}

//Colorize table
QVariant EnhancedSqlTableModel::data(const QModelIndex &item, int role) const
{
                                                                            //Colorize STATUS based on value
    if(item.column() == 5 && \
            role == Qt::TextColorRole && \
            item.data(Qt::DisplayRole).toString() == "ALIVE") {
        return QVariant::fromValue(QColor(0,192,0,255));
    }
    if(item.column() == 5 && \
            role == Qt::TextColorRole && \
            item.data(Qt::DisplayRole).toString() == "DEAD") {
        return QVariant::fromValue(QColor(192,0,0,255));
    }

    if(item.column() == 1 &&                                                //Highlight "Name" column
            role == Qt::TextColorRole)
        return QVariant::fromValue(QColor(0,0,192,255));


    return QSqlQueryModel::data(item, role);
}


