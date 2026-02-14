#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    deal_dialog = new dealDialog(this);
    deal_dialog->hide();


    server = new QTcpServer(this);
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    server->isListening();
    QList<QTcpSocket*> sockets = clientMap.keys();

    for (QTcpSocket* socket : sockets) {
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            socket->disconnectFromHost();
        }
    }

    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "确认", "确定要退出程序吗？",
                                                               QMessageBox::No | QMessageBox::Yes,
                                                               QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
        return;
    }

    event->accept();
}

void MainWindow::init()
{
    if (!server->listen(QHostAddress::Any, 8888)) {
        ui->msgFromClients_listWidget->addItem("启动服务器失败！");
    } else {
        ui->msgFromClients_listWidget->addItem("启动服务器成功！");
    }

    connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    ui->sort_box->addItem("按订单创建时间正序");
    ui->sort_box->addItem("按订单创建时间倒序");
    ui->sort_box->addItem("按报价大小正序");
    ui->sort_box->addItem("按报价大小倒序");

    ui->sort_box_2->addItem("按订单创建时间正序");
    ui->sort_box_2->addItem("按订单创建时间倒序");
    ui->sort_box_2->addItem("按报价大小正序");
    ui->sort_box_2->addItem("按报价大小倒序");

    connect(ui->sort_box, &QComboBox::currentTextChanged, this, &MainWindow::sortBoxChanged);
    connect(ui->sort_box_2, &QComboBox::currentTextChanged, this, &MainWindow::sortBoxChanged);
    connect(ui->pendingTransaction_listWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::selectedItem);
    connect(ui->finishedTransaction_listWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::selectedItem);
    connect(ui->metalPrice_frame, &interactableFrame::clicked, this, &MainWindow::onMetalPriceFrame);

    //check transaction directory
    QDir dir;
    if(!dir.exists("bin/transactions"))
        if (dir.mkpath("bin/transactions")) {
            qDebug() << "Dir created";
        }
    readLocalTransaction();
    sortBoxChanged(ui->sort_box->currentText());
    updateListWidget();
    updateLabel();

    QList<QLabel *> allLabels = this->findChildren<QLabel *>();

    for (QLabel *label : allLabels) {
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
}

void MainWindow::selectedItem(QListWidgetItem *item)
{
    if(!item) return;
    deal_dialog->show();
    QString filePath = item->data(Qt::UserRole).toString();
    transaction clickedData;

    qDebug()<<filePath;

    if(item->listWidget() == ui->pendingTransaction_listWidget)
    {
        auto it = std::find_if(fileVector.begin(), fileVector.end(), [=](transaction d){
            return d.selectFilePath() == filePath;
        });

        if (it == fileVector.end())return;

        clickedData = *it;
        deal_dialog->updateTransaction(clickedData);


    }
    else if(item->listWidget() == ui->finishedTransaction_listWidget)
    {
        auto it = std::find_if(finishedFileVector.begin(), finishedFileVector.end(), [=](transaction d){
            return d.selectFilePath() == filePath;
        });

        if (it == finishedFileVector.end())return;

        clickedData = *it;
        deal_dialog->updateTransaction(clickedData);
    }

    disconnect(deal_dialog, &dealDialog::finished, nullptr, nullptr);
    connect(deal_dialog, &dealDialog::finished,[=](bool isAccept)
    {
        TransactionHandled(clickedData, isAccept);
        deal_dialog->hide();
        deal_dialog->disableBtn();

        //refresh widget to show updated file info
        readLocalTransaction();
        sortBoxChanged(ui->sort_box->currentText());
        updateListWidget();
        updateLabel();
    });

}

void MainWindow::TransactionHandled(transaction data, bool isAccept)
{
    if(isAccept)
        data.toogleAccept();

    data.setResultTime(QDateTime::currentDateTime());
    QTcpSocket* socket = fetchSocketFromUuid(data.getUuid());
    if(socket == nullptr)
    {
        ui->msgFromClients_listWidget->addItem("发送结果失败，客户端不存在或已下线");
        return;
    }
    sendMsgToSocket(socket, TRANSACTION_STATUS, data);

    saveTransactionToLocal(data);
    readLocalTransaction();
    sortBoxChanged(ui->sort_box->currentText());
    updateListWidget();
    updateLabel();
}

void MainWindow::sortBoxChanged(QString way)
{
    ui->sort_box->setCurrentText(way);
    ui->sort_box_2->setCurrentText(way);
    if(way == "按订单创建时间正序")
    {
        std::sort(fileVector.begin(), fileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectSubmittedTime() > b.selectSubmittedTime();});

        std::sort(finishedFileVector.begin(), finishedFileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectSubmittedTime() > b.selectSubmittedTime();});
    }
    else if(way == "按订单创建时间倒序")
    {
        std::sort(fileVector.begin(), fileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectSubmittedTime() < b.selectSubmittedTime();});

        std::sort(finishedFileVector.begin(), finishedFileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectSubmittedTime() < b.selectSubmittedTime();});
    }
    else if(way == "按报价大小正序")
    {
        std::sort(fileVector.begin(), fileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectPrice() > b.selectPrice();});

        std::sort(finishedFileVector.begin(), finishedFileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectPrice() > b.selectPrice();});
    }
    else if(way == "按报价大小倒序")
    {
        std::sort(fileVector.begin(), fileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectPrice() < b.selectPrice();});

        std::sort(finishedFileVector.begin(), finishedFileVector.end(),
                  [=](transaction &a, transaction &b){return a.selectPrice() < b.selectPrice();});
    }
}

void MainWindow::updateListWidget()
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
    
    ui->finishedTransaction_listWidget->clear();
    for(auto data : finishedFileVector)
    {
        QListWidgetItem* item = new QListWidgetItem(data.getId() + "---" +
                                                    data.selectType() + "---" +
                                                    QString::number(data.selectPrice()));
        item->setData(Qt::UserRole, data.selectFilePath());
        ui->finishedTransaction_listWidget->addItem(item);
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
    finishedFileVector.clear();

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
        if(!data.selectResultTime().isValid())
            fileVector.push_back(data);
        else
            finishedFileVector.push_back(data);

        file.close();
    }
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
    ui->finishedTransactionNumber_label->setText(QString("当前订单数量:%1").arg(finishedFileVector.length()));
}

QTcpSocket* MainWindow::fetchSocketFromUuid(QString uuid)
{
    QMap<QTcpSocket*, clientInfo*>::iterator it = clientMap.begin();

    while (it != clientMap.end()) {
        if (it.value() && it.value()->getUuid() == uuid) {
            return it.key();
        }
        ++it;
    }
    return nullptr;
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

void MainWindow::onMetalPriceFrame()
{

}

//client-server function
void MainWindow::clientConnected()
{

}

void MainWindow::clientDisconnected()
{

}

void MainWindow::onNewConnection()
{
    QTcpSocket* socket = server->nextPendingConnection();

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
        in >> msg_type;

        if(msg_type == HANDSHAKE)
        {
            QString uuid;
            in >> uuid;
            qDebug()<<uuid;
            clientMap.insert(socket, new clientInfo(uuid));

            ui->msgFromClients_listWidget->addItem(QString(uuid + "已连接"));
        }
        else if(msg_type == NEW_TRANSACTION)
        {
            in >> data;
            if(!in.commitTransaction())
                return;
            sendMsgToSocket(socket, NEW_TRANSACTION, data);
            newTransactionRecived(data);

            ui->msgFromClients_listWidget->addItem
                    (QString(clientMap.value(socket)->getUuid() + "发送了一份订单"+ data.getId()));
        }

    });
}

void MainWindow::newTransactionRecived(transaction data)
{
    saveTransactionToLocal(data);
    readLocalTransaction();
    sortBoxChanged(ui->sort_box->currentText());
    updateListWidget();
    updateLabel();
}














