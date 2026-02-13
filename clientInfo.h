#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#include<QDebug>
#include<QDateTime>

class clientInfo
{
public:
    clientInfo(QString uuid);
    ~clientInfo();
    void setIp(QString ip);

    //quest
    QDateTime selectLoginTime();
    QString getIp();
    QString getUuid();

private:
    QDateTime loginTime;
    QString ip;
    QString uuid;

};

#endif // CLIENTINFO_H
