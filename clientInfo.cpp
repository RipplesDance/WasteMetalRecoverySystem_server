#include "clientInfo.h"

clientInfo::clientInfo(QString ip)
{
    loginTime = QDateTime::currentDateTime();
    this->ip = ip;
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
