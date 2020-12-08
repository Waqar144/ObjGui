#ifndef STRINGS_H
#define STRINGS_H

#include "QVector"

class Strings
{
public:
    Strings();
    void setStringsData(std::tuple<QVector<QByteArray>, QVector<QByteArray> >&& data);
    int getIndexByAddress(QByteArray address);
    QString getAddressAt(int index);
    QString getStringAt(int index);
    QString getStringsAddresses();
    QString getStrings();
private:
    QVector<qint64> addrsLong;
    QVector<QByteArray> strings;
    QVector<QByteArray> addresses;
    int matrixLen;
};

#endif // STRINGS_H
