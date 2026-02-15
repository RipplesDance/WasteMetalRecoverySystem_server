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

    ui->sort_box_2->addItem("按订单创建时间正序");
    ui->sort_box_2->addItem("按订单创建时间倒序");
    ui->sort_box_2->addItem("按报价大小正序");
    ui->sort_box_2->addItem("按报价大小倒序");

    //ui connection
    connect(ui->sort_box, &QComboBox::currentTextChanged, this, &MainWindow::sortBoxChanged);
    connect(ui->sort_box_2, &QComboBox::currentTextChanged, this, &MainWindow::sortBoxChanged);
    connect(ui->pendingTransaction_listWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::selectedItem);
    connect(ui->finishedTransaction_listWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::selectedItem);

    connect(ui->metalPrice_frame, &interactableFrame::clicked, this, &MainWindow::onMetalPriceFrame);
    connect(ui->onlineClients_frame, &interactableFrame::clicked, this, &MainWindow::onOnlineClientsFrame);

    //dialog init
    deal_dialog = new dealDialog(this);
    deal_dialog->hide();

    metalPrice_dialog = new metalPriceDialog(this);
    metalPrice_dialog->hide();
    connect(metalPrice_dialog, &metalPriceDialog::update, this, &MainWindow::updateMetalPrice);

    onlineClients_dialog = new onlineClientsDialog(this);
    onlineClients_dialog->hide();
    connect(onlineClients_dialog,&onlineClientsDialog::heartBeat,this,&MainWindow::heartBeatDetection);

    init();
    //load quotation model
    readQuotationModel();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "确认", "确定要退出程序吗？",
                                                               QMessageBox::No | QMessageBox::Yes,
                                                               QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
        return;
    }
    addMsgToMsgServer("服务器关闭");
    server->isListening();
    QList<QTcpSocket*> sockets = clientMap.keys();

    for (QTcpSocket* socket : sockets) {
        if (socket && socket->state() == QAbstractSocket::ConnectedState) {
            socket->disconnectFromHost();
        }
    }

    logFile.close();

    event->accept();
}

void MainWindow::init()
{
    //check transaction directory
    QDir dir;
    if(!dir.exists("bin/transactions"))
        if (dir.mkpath("bin/transactions")) {
            qDebug() << "Dir created";
        }
    //check logs directory
    if(!dir.exists("bin/logs"))
        if (dir.mkpath("bin/logs")) {
            qDebug() << "Dir created";
        }

    //check quotation model directory
    if (!dir.exists("bin/quotation_model")) dir.mkpath("bin/quotation_model");

    readLocalTransaction();
    sortBoxChanged(ui->sort_box->currentText());
    updateTransactionListWidget();
    updateLabel();

    logFilePath = QString("bin/logs/%1.log").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    logFile.setFileName(logFilePath);
    if(!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        addMsgToMsgServer("无法打开日志文件，本次启动的数据将无法保存到本地!");
    }
    writeLog("=================服务器开始运行===================");

    QList<QLabel *> allLabels = this->findChildren<QLabel *>();

    for (QLabel *label : allLabels) {
        label->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    server = new QTcpServer(this);
    if (!server->listen(QHostAddress::Any, 8888)) {
        addMsgToMsgServer("启动服务器失败！");
    } else {
        addMsgToMsgServer("启动服务器成功！");
    }
    connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

}

void MainWindow::addMsgToMsgServer(QString str)
{
    writeLog(getCurrentDateTime() + ": " + str);
    ui->msgFromClients_listWidget->insertItem(0,getCurrentDateTime() + ": " + str);
}

void MainWindow::writeLog(QString msg) {
        QTextStream out(&logFile);
        out.setCodec("UTF-8");
        out << msg << "\n";
}

void MainWindow::selectedItem(QListWidgetItem *item)
{
    if(!item) return;
    deal_dialog->show();
    QString filePath = item->data(Qt::UserRole).toString();
    transaction clickedData;

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
        updateTransactionListWidget();
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
        ui->msgFromClients_listWidget->insertItem(0,getCurrentDateTime()+": "+"发送结果失败，客户端不存在或已下线");
        return;
    }
    sendMsgToSocket(socket, TRANSACTION_STATUS, data);

    saveTransactionToLocal(data);
    readLocalTransaction();
    sortBoxChanged(ui->sort_box->currentText());
    updateTransactionListWidget();
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

void MainWindow::updateTransactionListWidget()
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
            addMsgToMsgServer("本地订单无法读取!");
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

        if(socket->write(block) == -1)
            socket->abort();
}

void MainWindow::sendMsgToSocket(QTcpSocket* socket, int msg_type, metalPrice data)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) return;

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_14);

        out<<msg_type;
        if(msg_type == METAL_PRICE)
        {
            out<<data;
        }

        if(socket->write(block) == -1)
            socket->abort();
}

void MainWindow::sendMsgToSocket(QTcpSocket* socket, int msg_type)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_14);

    out<<msg_type;

    if(socket->write(block) == -1)
        socket->abort();
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
        addMsgToMsgServer("无法打开本地订单文件:"+data.getId());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_14);
    out << data;
    file.close();
}

void MainWindow::onMetalPriceFrame()
{
    metalPrice_dialog->show();
}

void MainWindow::onOnlineClientsFrame()
{
    onlineClients_dialog->show();
    onlineClients_dialog->updateClientNumber(clientMap.count());
    onlineClients_dialog->clearTreeWidget();

    QMap<QTcpSocket*, clientInfo*>::iterator it;

    for (it = clientMap.begin(); it != clientMap.end(); ++it) {
        clientInfo *info = it.value();
        onlineClients_dialog->addClient(info);
    }

}

void MainWindow::heartBeatDetection()
{
    QMap<QTcpSocket*, clientInfo*>::iterator it = clientMap.begin();
        while (it != clientMap.end()) {
            QTcpSocket* socket = it.key();
            clientInfo* info = it.value();
            sendMsgToSocket(socket,HEART_BEAT);
            info->testHeartBeat();
            it++;
        }
}

void MainWindow::removeZombie(clientInfo* data)
{
    QTcpSocket* socket = fetchSocketFromUuid(data->getUuid());
    if(!socket)
        return;
    socket->abort();

    onlineClients_dialog->updateClientNumber(clientMap.count());
    onlineClients_dialog->clearTreeWidget();

    QMap<QTcpSocket*, clientInfo*>::iterator it;

    for (it = clientMap.begin(); it != clientMap.end(); ++it) {
        clientInfo *info = it.value();
        onlineClients_dialog->addClient(info);
    }
}

void MainWindow::updateMetalPrice(metalPrice data)
{
    saveMetalPriceToLocal(data);

    QMap<QTcpSocket*, clientInfo*>::iterator it;

    for (it = clientMap.begin(); it != clientMap.end(); ++it) {
        QTcpSocket *socket = it.key();
        sendMsgToSocket(socket,METAL_PRICE, data);
    }

    addMsgToMsgServer("已向所有客户端发送最新金属价格");
    metalPrice_dialog->hide();
}

metalPrice MainWindow::readMetalPriceFromLocal()
{
    metalPrice data;

    QFile file("bin/metalPrice_CNY.dat");
    if(!file.open(QIODevice::ReadOnly))
    {
        addMsgToMsgServer("无法读取本地金属价格信息!");
        return data;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_14);
    in >> data;
    return data;
}

void MainWindow::saveMetalPriceToLocal(metalPrice data)
{

    QFile file("bin/metalPrice_CNY.dat");
    if(!file.open(QIODevice::WriteOnly))
    {
        addMsgToMsgServer("无法保存本地金属价格信息!");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_14);
    out << data;
}

QString MainWindow::getCurrentDateTime()
{
    QDateTime time = QDateTime::currentDateTime();
    return time.toString("yyyy-MM-dd hh-mm");
}

void MainWindow::saveQuotationToLocal()
{
    QFile file("bin/quotation_model/quotation.dat");
    if(!file.open(QIODevice::WriteOnly))
    {
        addMsgToMsgServer("无法保存核心报价模块!");
        return;
    }
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_14);
    out << quo;

    quo.saveBatteryMaterialConcentrationToLocal();

    file.close();
}

void MainWindow::readQuotationModel()
{
    QFile file("bin/quotation_model/quotation.dat");
    if(!file.open(QIODevice::ReadOnly))
    {
        addMsgToMsgServer("无法读取核心报价模块!");
        return;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_14);
    in >> quo;
    file.close();
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
        if (clientMap.contains(socket)) {
                clientInfo* info = clientMap.value(socket);
                QString uuid = info->getUuid();

                addMsgToMsgServer(uuid + " 断开连接");

                clientMap.remove(socket);
                delete info;
            }
            socket->deleteLater();
    });

    //message event
    connect(socket, &QTcpSocket::readyRead, [=](){messageFromClient(socket);});
}

void MainWindow::newTransactionRecived(transaction data)
{
    saveTransactionToLocal(data);
    readLocalTransaction();
    sortBoxChanged(ui->sort_box->currentText());
    updateTransactionListWidget();
    updateLabel();
}

void MainWindow::messageFromClient(QTcpSocket* socket)
{
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
        if(!in.commitTransaction())
        {
            addMsgToMsgServer("客户端消息接收失败！");
            return ;
        }
        clientInfo* client = new clientInfo(uuid);
        connect(client, &clientInfo::zombie,this, [=](){
            removeZombie(client);
        });

        //IPv6 to IPv4
        QHostAddress hostAddr = socket->peerAddress();
        QString ipString;
        if (hostAddr.protocol() == QAbstractSocket::IPv4Protocol)
            ipString = hostAddr.toString();
        else
        {
            bool success;
            uint ipv4 = hostAddr.toIPv4Address(&success);
            if (success)
                ipString = QHostAddress(ipv4).toString();
            else
                ipString = hostAddr.toString();
        }

        client->setIp(ipString);
        clientMap.insert(socket, client);

        addMsgToMsgServer(QString(uuid + "已连接"));

        //convey metalPrice
        metalPrice data = readMetalPriceFromLocal();
        if(data.isUpdated)
            sendMsgToSocket(socket,METAL_PRICE,data);
    }
    else if(msg_type == NEW_TRANSACTION)
    {
        in >> data;
        if(!in.commitTransaction())
            return;
        sendMsgToSocket(socket, NEW_TRANSACTION, data);
        newTransactionRecived(data);

        addMsgToMsgServer(QString("收到了一份订单"+ data.getId()));
    }
    else if(msg_type == HEART_BEAT)
    {
        clientInfo* client = clientMap.value(socket);
        client->heartBeat();
    }
}

