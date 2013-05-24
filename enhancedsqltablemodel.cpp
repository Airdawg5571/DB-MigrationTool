#include "enhancedsqltablemodel.h"
#include <QColor>

EnhancedSqlTableModel::EnhancedSqlTableModel(QObject *parent) :
    QSqlRelationalTableModel(parent)
{   
}

//Set STATUS column as not editable
Qt::ItemFlags EnhancedSqlTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlTableModel::flags(index);
    if ((index.column() == 5)) flags &=~Qt::ItemIsEnabled; //Not editable by user, but still changeable
    return flags;
}

//Color STATUS column based on value
QVariant EnhancedSqlTableModel::data(const QModelIndex &item, int role) const
{
    QVariant value = QSqlQueryModel::data(item,role);
    if(item.column() == 5 && \
            role == Qt::TextColorRole && \
            item.data(Qt::DisplayRole).toString() == "ALIVE") {
        return qVariantFromValue(QColor(0,192,0,255));
    }
    if(item.column() == 5 && \
            role == Qt::TextColorRole && \
            item.data(Qt::DisplayRole).toString() == "DEAD") {
        return qVariantFromValue(QColor(192,0,0,255));
    }
    return value;
}
