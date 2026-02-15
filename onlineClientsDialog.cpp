#include "onlineClientsDialog.h"
#include "ui_onlineClientsDialog.h"

onlineClientsDialog::onlineClientsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::onlineClientsDialog)
{
    ui->setupUi(this);
}

onlineClientsDialog::~onlineClientsDialog()
{
    delete ui;
}

void onlineClientsDialog::addClient(clientInfo data)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->clientsInfo_treeWidget);

    item->setText(0,data.getIp());
    item->setText(1,data.getUuid());
    item->setText(2,data.selectLoginTime().toString("yyyy-MM-dd hh:mm:ss"));
}

void onlineClientsDialog::updateClientNumber(int n)
{
    ui->clientsNumber_label->setText(QString("当前在线人数:%1").arg(n));
}

void onlineClientsDialog::clearTreeWidget()
{
    ui->clientsInfo_treeWidget->clear();
}
