#ifndef STRINGSDUMPER_H
#define STRINGSDUMPER_H

#include "QString"
#include <QPair>
#include <tuple>

class StringsDumper
{
public:
    StringsDumper() = default;
    std::tuple<QVector<QByteArray>, QVector<QByteArray>> dumpStrings(QString file, const QVector<QString>& baseOffsets);
private:
    QByteArray getAddressFromOffset(qint64 offset, qint64 baseAddr, qint64 baseOffset);
};

#endif // STRINGSDUMPER_H
