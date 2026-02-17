#ifndef RECOVERYCOST_H
#define RECOVERYCOST_H

#include<QDebug>
#include<QDataStream>

class recoveryCost
{
public:
    recoveryCost();
    ~recoveryCost();

    void setProperty(double unitPrice_80, double unitPrice_90, double price_per_kilo, double profit);

    friend QDataStream &operator<<(QDataStream &out, const recoveryCost &data);
    friend QDataStream &operator>>(QDataStream &in, recoveryCost &data);

public:
    //property
    double unitPrice_80;//CNY/Wh. Depends on market
    double unitPrice_90;//CNY/Wh. Depends on market

    double price_per_kilo; //include electricity fee， labor fee, and chemicals fee. Count in CNY.
    double profit;
};

#endif // RECOVERYCOST_H
