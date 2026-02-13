#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#include<QDebug>
#include<QDateTime>

class clientInfo
{
public:
    clientInfo(QString ip = NULL);
    ~clientInfo();
    void setIp(QString ip);

    //quest
    QDateTime selectLoginTime();
    QString getIp();

private:
    QDateTime loginTime;
    QString ip;
};

#endif // CLIENTINFO_H
