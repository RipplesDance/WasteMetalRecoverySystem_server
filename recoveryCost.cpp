#include "recoveryCost.h"

recoveryCost::recoveryCost()
{
    this->unitPrice_80 = 0;
    this->unitPrice_90 = 0;

    this->price_per_kilo = 0;
    this->profit = 0;
}
recoveryCost::~recoveryCost(){}

void recoveryCost::setProperty(double unitPrice_80, double unitPrice_90,
                               double price_per_kilo, double profit)
{
    this->unitPrice_80 = unitPrice_80;
    this->unitPrice_90 = unitPrice_90;

    this->price_per_kilo = price_per_kilo;
    this->profit = profit;
}

QDataStream &operator<<(QDataStream &out, const recoveryCost &data)
{
    out<<data.unitPrice_80<<data.unitPrice_90<<data.price_per_kilo<<data.profit;
    return out;
}

QDataStream &operator>>(QDataStream &in, recoveryCost &data)
{
    in >>data.unitPrice_80>>data.unitPrice_90>>data.price_per_kilo>>data.profit;
    return in;
}
