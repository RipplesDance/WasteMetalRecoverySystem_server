#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include<QMessageBox>
#include<QMap>
#include<QDir>
#include<QDataStream>
#include<QFile>
#include<QListWidgetItem>
#include<QVector>
#include"transaction.h"
#include"clientInfo.h"

enum {
    NEW_TRANSACTION = 1,
    TRANSACTION_STATUS = 2,
    METAL_PRICE = 3,
    QUOTATION_CHANGE = 4
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void init();
    void newTransactionRecived(transaction data);
    void saveTransactionToLocal(transaction data);
    void sendMsgToSocket(QTcpSocket* socket, int msg_type, transaction data);
    void updateLabel();
    void readLocalTransaction();
    void updataListWidget();

public slots:
    void clientConnected();
    void clientDisconnected();
    void onNewConnection();
    void sortBoxChanged(QString way);
    void selectedItem(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QTcpServer *server;
    QMap<QTcpSocket*, clientInfo*> clientMap;
    QVector<transaction> fileVector;

};
#endif // MAINWINDOW_H
