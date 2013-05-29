#ifndef CHANGEDBS_H
#define CHANGEDBS_H

#include <QDialog>
#include <QSqlDatabase>

namespace Ui {
class ChangeDBs;
}

class ChangeDBs : public QDialog
{
    Q_OBJECT
    
public:
    explicit ChangeDBs(QWidget *parent = 0);
    ~ChangeDBs();

    QStringList drivers=QSqlDatabase::drivers();

    QString name() const;
    QString dbName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    QString driver() const;
    int port() const;

    void setName(QString name);
    void setDbName(QString dbName);
    void setUserName(QString userName);
    void setPassword(QString password);
    void setHostName(QString hostName);
    void setDriver(QString driver);
    void setPort(QString port);
private:
    Ui::ChangeDBs *ui;


private slots:
    void setupPlaceholders(QString driver);
    void resetAction();
    void okAction();
};

#endif // CHANGEDBS_H
