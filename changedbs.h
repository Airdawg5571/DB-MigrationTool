#ifndef CHANGEDBS_H
#define CHANGEDBS_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSettings>
#include <QMessageBox>
#include <QtSql>

namespace Ui {
class ChangeDBs;
}

class ChangeDBs : public QDialog
{
    Q_OBJECT
    
public:
    explicit ChangeDBs(QWidget *parent = 0);
    ~ChangeDBs();

    QStringList drivers;
    QStringList odbcDrivers;
    QSqlDatabase testDb;

    QString name() const;
    QString dbName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    QString driver() const;
    QString options() const;
    QString port() const;

    void setName(QString name);
    void setDbName(QString dbName);
    void setUserName(QString userName);
    void setPassword(QString password);
    void setHostName(QString hostName);
    void setDriver(QString driver);
    void setOptions(QString option);
    void setPort(QString port);

public slots:
    void overrideDSN(bool ovr);

private:
    Ui::ChangeDBs *ui;

private slots:
    void setupLayout(QString driver);
    void clearAction();
    void okAction();
    void testAction();
};

#endif // CHANGEDBS_H
