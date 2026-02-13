#include "clientInfo.h"

clientInfo::clientInfo(QString uuid)
{
    loginTime = QDateTime::currentDateTime();
    this->uuid = uuid;
}

clientInfo::~clientInfo()
{

}

void clientInfo::setIp(QString ip)
{
    this->ip = ip;
}

QDateTime clientInfo::selectLoginTime(){return loginTime;}
QString clientInfo::getIp(){return ip;}
QString clientInfo::getUuid(){return uuid;}
