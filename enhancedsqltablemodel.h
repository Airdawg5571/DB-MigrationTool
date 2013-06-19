#ifndef ENHANCEDSQLTABLEMODEL_H
#define ENHANCEDSQLTABLEMODEL_H

#include <QSqlRelationalTableModel>

class EnhancedSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit EnhancedSqlTableModel(QObject *parent = 0);
    Qt::ItemFlags flags(const QModelIndex &index) const;
signals:
    
public slots:
    
};

#endif // ENHANCEDSQLTABLEMODEL_H
