#ifndef CHANGEDBS_H
#define CHANGEDBS_H

#include <QDialog>

namespace Ui {
class ChangeDBs;
}

class ChangeDBs : public QDialog
{
    Q_OBJECT
    
public:
    explicit ChangeDBs(QWidget *parent = 0);
    ~ChangeDBs();
    
private:
    Ui::ChangeDBs *ui;
};

#endif // CHANGEDBS_H
