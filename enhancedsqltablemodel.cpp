#include "enhancedsqltablemodel.h"
#include <QColor>



EnhancedSqlTableModel::EnhancedSqlTableModel(QObject *parent) :
    QSqlTableModel(parent)
{   
}

QVariantList EnhancedSqlTableModel::getValuesAt(int row, std::initializer_list<int> colIndices)
{
    QVariantList result;
    result.reserve(args.size());
    for (int col: args)
        result.append(index(row, col).data());
    return result;
}

//Set STATUS column as not editable
Qt::ItemFlags EnhancedSqlTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QSqlTableModel::flags(index);
    if ((index.column() == 9)) flags &=~Qt::ItemIsEnabled;                  //Not editable by user, but still changeable
    return flags;
}
