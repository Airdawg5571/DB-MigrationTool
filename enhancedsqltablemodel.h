#ifndef ENHANCEDSQLTABLEMODEL_H
#define ENHANCEDSQLTABLEMODEL_H

#include <QSqlRelationalTableModel>

class EnhancedSqlTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT
public:
    explicit EnhancedSqlTableModel(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // ENHANCEDSQLTABLEMODEL_H
