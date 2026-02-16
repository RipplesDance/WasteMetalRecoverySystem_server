#include "batteryDialog.h"
#include "ui_batteryDialog.h"

batteryDialog::batteryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::batteryDialog)
{
    ui->setupUi(this);
    init();
    connect(ui->batteryInfo_treeWidget, &QTreeWidget::customContextMenuRequested, this, &batteryDialog::showContextMenu);
    connect(ui->batteryInfo_treeWidget, &QTreeWidget::itemChanged, this, &batteryDialog::handleItemChanged);
}

void batteryDialog::init()
{
    for(int i = 0; i < ui->batteryInfo_treeWidget->columnCount(); ++i) {
            ui->batteryInfo_treeWidget->resizeColumnToContents(i);
        }
    ui->batteryInfo_treeWidget->setColumnWidth(0,200);
}

void batteryDialog::handleItemChanged(QTreeWidgetItem *item)
{
    qDebug()<<"itemChanged";
    QString key = item->text(0);
    batteryMaterialConcentration* value = fetchDataFromItem(item);
    batteryChanged(key, value);
}

batteryMaterialConcentration* batteryDialog::fetchDataFromItem(QTreeWidgetItem *item)
{
    batteryMaterialConcentration* data = new batteryMaterialConcentration;
    data->positiveMaterialsRatio = item->text(1).toDouble();
    data->positiveMaterial_recycleRatio = item->text(2).toDouble();
    data->compoundRatio = item->text(3).toDouble();
    data->compound_recycleRatio = item->text(4).toDouble();
    data->li = item->text(5).toDouble();
    data->li_recycleRatio = item->text(6).toDouble();
    data->co = item->text(7).toDouble();
    data->co_recycleRatio = item->text(8).toDouble();
    data->ni = item->text(9).toDouble();
    data->ni_recycleRatio = item->text(10).toDouble();
    data->mn = item->text(11).toDouble();
    data->mn_recycleRatio = item->text(12).toDouble();
    data->cu = item->text(13).toDouble();
    data->cu_recycleRatio = item->text(14).toDouble();
    return data;
}

void batteryDialog::showContextMenu(QPoint pos)
{
        QMenu menu(this);

        QAction *addAction = menu.addAction("添加新电池材料");

        QTreeWidgetItem *curItem = ui->batteryInfo_treeWidget->itemAt(pos);
        QAction *delAction = nullptr;

        if (curItem) {
            delAction = menu.addAction("删除当前电池材料");
        }

        QAction *selectedAction = menu.exec(QCursor::pos());

        if (selectedAction == addAction) {
            addNewItem();
        } else if (curItem && selectedAction == delAction) {
            deleteSelectedItem(curItem);
        }
}

void batteryDialog::addNewItem() {
    QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->batteryInfo_treeWidget);
    newItem->setText(0, "新电池材料");
    newItem->setText(1, QString::number(0)+"%");
    newItem->setText(2, QString::number(0)+"%");
    newItem->setText(3, QString::number(0)+"%");
    newItem->setText(4, QString::number(0)+"%");
    newItem->setText(5, QString::number(0)+"%");
    newItem->setText(6, QString::number(0)+"%");
    newItem->setText(7, QString::number(0)+"%");
    newItem->setText(8, QString::number(0)+"%");
    newItem->setText(9, QString::number(0)+"%");
    newItem->setText(10, QString::number(0)+"%");
    newItem->setText(11, QString::number(0)+"%");
    newItem->setText(12, QString::number(0)+"%");
    newItem->setText(13, QString::number(0)+"%");
    newItem->setText(14, QString::number(0)+"%");

    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);

    ui->batteryInfo_treeWidget->addTopLevelItem(newItem);
    ui->batteryInfo_treeWidget->editItem(newItem, 0); // edit immediately
}

void batteryDialog::deleteSelectedItem(QTreeWidgetItem *item) {
    if (!item) return;

    QString key = item->text(0);

    auto result = QMessageBox::question(this, "确认删除", QString("确定要删除电池材料 [%1] 吗？").arg(key));
    if (result == QMessageBox::Yes) {

        emit removeBattery(key);
        delete item;
    }
}

void batteryDialog::clearTreeWidget()
{
    ui->batteryInfo_treeWidget->clear();
}

void batteryDialog::addItemToTreeWidget(QString key, batteryMaterialConcentration* data)
{
    ui->batteryInfo_treeWidget->blockSignals(true);

    QTreeWidgetItem* item = new QTreeWidgetItem(ui->batteryInfo_treeWidget);
    item->setText(0, key);
    item->setText(1, QString::number(data->positiveMaterialsRatio*100)+"%");
    item->setText(2, QString::number(data->positiveMaterial_recycleRatio*100)+"%");
    item->setText(3, QString::number(data->compoundRatio*100)+"%");
    item->setText(4, QString::number(data->compound_recycleRatio*100)+"%");
    item->setText(5, QString::number(data->li*100)+"%");
    item->setText(6, QString::number(data->li_recycleRatio*100)+"%");
    item->setText(7, QString::number(data->co*100)+"%");
    item->setText(8, QString::number(data->co_recycleRatio*100)+"%");
    item->setText(9, QString::number(data->ni*100)+"%");
    item->setText(10, QString::number(data->ni_recycleRatio*100)+"%");
    item->setText(11, QString::number(data->mn*100)+"%");
    item->setText(12, QString::number(data->mn_recycleRatio*100)+"%");
    item->setText(13, QString::number(data->cu*100)+"%");
    item->setText(14, QString::number(data->cu_recycleRatio*100)+"%");

    item->setFlags(item->flags() | Qt::ItemIsEditable);

    ui->batteryInfo_treeWidget->blockSignals(false);

}

batteryDialog::~batteryDialog()
{
    delete ui;
}
