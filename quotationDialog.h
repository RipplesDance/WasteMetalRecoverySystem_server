#ifndef QUOTATIONDIALOG_H
#define QUOTATIONDIALOG_H

#include <QDialog>
#include<QDebug>

namespace Ui {
class quotationDialog;
}

class quotationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit quotationDialog(QWidget *parent = nullptr);
    ~quotationDialog();

private:
    Ui::quotationDialog *ui;
};

#endif // QUOTATIONDIALOG_H
