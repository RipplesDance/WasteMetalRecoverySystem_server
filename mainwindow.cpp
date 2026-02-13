#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->sort_box->addItem("按订单创建时间正序");
    ui->sort_box->addItem("按订单创建时间倒序");
    ui->sort_box->addItem("按报价大小正序");
    ui->sort_box->addItem("按报价大小倒序");

    connect(ui->sort_box, &QComboBox::currentTextChanged, this, &MainWindow::sortBoxChanged);
    connect(ui->pendingTransaction_listWidget, &QListWidget::itemClicked, this, &MainWindow::selectedItem);

    server = new QTcpServer(this);
//    connect(server,)
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    if (!server->listen(QHostAddress::Any, 8888)) {
        QMessageBox::critical(this, "错误", QString("启动服务器失败！" + server->errorString()));
    } else {
        QMessageBox::information(this, "成功", "启动服务器成功！");
    }

    connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    //check transaction directory
    QDir dir;
    if(!dir.exists("bin/transactions"))
        if (dir.mkpath("bin/transactions")) {
            qDebug() << "Dir created";
        }
    readLocalTransaction();
    updateLabel();
}

void MainWindow::selectedItem(QListWidgetItem *item)
{
    //
}

void MainWindow::sortBoxChanged(QString way)
{
    if(way == "按订单创建时间正序")
    {
        std::sort(fileVector.begin(), fileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectSubmittedTime() > b.selectSubmittedTime();});
    }
    else if(way == "按订单创建时间倒序")
    {
        std::sort(fileVector.begin(), fileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectSubmittedTime() < b.selectSubmittedTime();});
    }
    else if(way == "按报价大小正序")
    {
        std::sort(fileVector.begin(), fileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectPrice() > b.selectPrice();});
    }
    else if(way == "按报价大小倒序")
    {
        std::sort(fileVector.begin(), fileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectPrice() < b.selectPrice();});
    }

    updataListWidget();
}

void MainWindow::updataListWidget()
{
    ui->pendingTransaction_listWidget->clear();
    for(auto data : fileVector)
    {
        QListWidgetItem* item = new QListWidgetItem(data.getId() + "---" +
                                                    data.selectType() + "---" +
                                                    QString::number(data.selectPrice()));
        item->setData(Qt::UserRole, data.selectFilePath());
        ui->pendingTransaction_listWidget->addItem(item);
    }
}

void MainWindow::readLocalTransaction()
{
    QDir dir("bin/transactions");

    //set filter
    QStringList filter;
    filter<< "*.dat";
    dir.setNameFilters(filter);

    //start reading file
    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDot | QDir::NoDotDot);

    fileVector.clear();
    for(const QFileInfo &fileInfo : fileList)
    {
        QFile file(fileInfo.absoluteFilePath());
        if(!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this,"错误","文件无法读取!");
            return;
        }
        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_5_14);
        transaction data;
        in >> data;
        if(!data.checkStatus())
            fileVector.push_back(data);

        file.close();
    }
    sortBoxChanged(ui->sort_box->currentText());
}

void MainWindow::sendMsgToSocket(QTcpSocket* socket, int msg_type, transaction data)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) return;

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_14);

        out<<msg_type;
        if(msg_type == TRANSACTION_STATUS)
        {
            out<<data;
        }

        socket->write(block);
}

void MainWindow::updateLabel()
{
    ui->transactionNumber_label->setText(QString("当前订单数量:%1").arg(fileVector.length()));
}

void MainWindow::clientConnected()
{

}

void MainWindow::clientDisconnected()
{

}

void MainWindow::onNewConnection()
{
    QTcpSocket* socket = server->nextPendingConnection();
    clientMap.insert(socket, new clientInfo(socket->peerAddress().toString()));

    //disconnect event
    connect(socket, &QTcpSocket::disconnected, [=](){
        clientMap.remove(socket);
        socket->deleteLater();
    });

    //message event
    connect(socket, &QTcpSocket::readyRead, [=](){
        QDataStream in(socket);
        in.setVersion(QDataStream::Qt_5_14);
        in.startTransaction();

        transaction data;
        int msg_type;
        in >> msg_type >> data;
        if(!in.commitTransaction())
            return;
        if(msg_type == NEW_TRANSACTION)
        {
            sendMsgToSocket(socket, NEW_TRANSACTION, data);
            newTransactionRecived(data);
        }

    });
}

void MainWindow::newTransactionRecived(transaction data)
{
    saveTransactionToLocal(data);
    readLocalTransaction();
}

void MainWindow::saveTransactionToLocal(transaction data)
{
    QFile file(data.selectFilePath());
    if(!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this,"警告",QString("无法打开文件!" + data.selectFilePath()));
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_14);
    out << data;
    file.close();
}












