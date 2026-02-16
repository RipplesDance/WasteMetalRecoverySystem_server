#ifndef BATTERYDIALOG_H
#define BATTERYDIALOG_H

#include <QDialog>
#include<QTreeWidgetItem>
#include<QPoint>
#include<QMenu>
#include<QMessageBox>
#include"batteryMaterialConcentration.h"

namespace Ui {
class batteryDialog;
}

class batteryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit batteryDialog(QWidget *parent = nullptr);
    ~batteryDialog();
    void init();
    void addItemToTreeWidget(QString key, batteryMaterialConcentration* data);
    void clearTreeWidget();
    void addNewItem();
    void deleteSelectedItem(QTreeWidgetItem* item);
    void handleItemChanged(QTreeWidgetItem *item);
    batteryMaterialConcentration* fetchDataFromItem(QTreeWidgetItem *item);

public slots:
    void showContextMenu(QPoint pos);

signals:
    void removeBattery(QString key);
    void batteryChanged(QString key, batteryMaterialConcentration* value);

private:
    Ui::batteryDialog *ui;
};

#endif // BATTERYDIALOG_H
