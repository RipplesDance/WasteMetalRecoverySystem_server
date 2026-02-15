#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include<QMessageBox>
#include<QMap>
#include<QList>
#include<QDir>
#include<QDataStream>
#include<QDateTime>
#include<QFile>
#include<QListWidgetItem>
#include<QVector>
#include<QCloseEvent>
#include"transaction.h"
#include"clientInfo.h"
#include"dealDialog.h"
#include"metalPrice.h"
#include"metalPriceDialog.h"
#include"onlineClientsDialog.h"

enum {
    HANDSHAKE = 0,
    NEW_TRANSACTION = 1,
    TRANSACTION_STATUS = 2,
    METAL_PRICE = 3,
    QUOTATION_CHANGE = 4,
    HEART_BEAT = 5
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
    void sendMsgToSocket(QTcpSocket* socket, int msg_type, metalPrice data);
    void sendMsgToSocket(QTcpSocket* socket, int msg_type);
    void updateLabel();
    void readLocalTransaction();
    void updateTransactionListWidget();
    QTcpSocket* fetchSocketFromUuid(QString uuid);
    void TransactionHandled(transaction data, bool isAccept);
    metalPrice readMetalPriceFromLocal();
    void saveMetalPriceToLocal(metalPrice data);
    QString getCurrentDateTime();
    void addMsgToMsgServer(QString str);
    void writeLog(QString msg);
    void messageFromClient(QTcpSocket* socket);

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void clientConnected();
    void clientDisconnected();
    void onNewConnection();
    void sortBoxChanged(QString way);
    void selectedItem(QListWidgetItem *item);
    void onMetalPriceFrame();
    void onOnlineClientsFrame();
    void updateMetalPrice(metalPrice data);
    void heartBeatDetection();
    void removeZombie(clientInfo data);

private:
    Ui::MainWindow *ui;
    QTcpServer *server;
    QMap<QTcpSocket*, clientInfo*> clientMap;
    QVector<transaction> fileVector;
    QVector<transaction> finishedFileVector;

    dealDialog* deal_dialog;
    metalPriceDialog* metalPrice_dialog;
    onlineClientsDialog* onlineClients_dialog;

    QString logFilePath;
    QFile logFile;

};
#endif // MAINWINDOW_H
