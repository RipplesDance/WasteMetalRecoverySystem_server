#include "clientInfo.h"

clientInfo::clientInfo(QString uuid)
{
    loginTime = QDateTime::currentDateTime();
    this->uuid = uuid;
    heartBeatCount = 0;
    heartBeatTimer = new QTimer;
    connect(heartBeatTimer, &QTimer::timeout,this, [=](){
        heartBeatTimer->stop();
        emit zombie();
    });
}

clientInfo::~clientInfo()
{

}

void clientInfo::setIp(QString ip)
{
    this->ip = ip;
}

void clientInfo::heartBeat()
{
    qDebug()<<"客户端存活";
    heartBeatTimer->stop();
}

void clientInfo::testHeartBeat()
{
    qDebug()<<"测试客户端心跳";
    heartBeatTimer->start(1000 * 15);
}

QDateTime clientInfo::selectLoginTime(){return loginTime;}
QString clientInfo::getIp(){return ip;}
QString clientInfo::getUuid(){return uuid;}
