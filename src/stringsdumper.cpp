#include "stringsdumper.h"

#include "QFile"
#include "QVector"

#include <QDebug>

// Extract strings from file along with their file offset and vma
std::tuple<QVector<QByteArray>, QVector<QByteArray>>
StringsDumper::dumpStrings(QString filename, const QVector<QString> &baseOffsets){
    QVector<QByteArray> strings;
    QVector<QByteArray> adresses;

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
        return {};

    // Seek to start of first section
    bool ok{};
    const qint64 startAddr = baseOffsets[0].toLongLong(&ok, 16);
    qint64 startPos = baseOffsets[1].toLongLong(&ok, 16);
    if (!ok)
        startPos = 0;

    const auto bytes = file.readAll();
    for (int i = startPos; i < bytes.length(); ++i) {
        char c = bytes[i];

        if (!isprint(c))
            continue;

        int pos = i;

        QByteArray str;
        str.reserve(12);
        str.append(c);

        for (int j = pos; j < bytes.length(); j++) {
            if (isprint(bytes[j])) {
                char b = bytes[j];
                str.append(b);
                i++;
            } else {
                i = j;
                break;
            }
        }

        if(str.length() >= 4){
            strings.append(str);
            auto res = getAddressFromOffset(pos - 1, startAddr, startPos);
            adresses.append(QByteArrayLiteral("0x") + res);
        }
    }

    return {strings, adresses};
}

// Return virtual memory address of file offset given base offsets
QByteArray StringsDumper::getAddressFromOffset(qint64 offset, qint64 baseAddr, qint64 baseOffset) {

    qlonglong offsetFromBase = offset - baseOffset;
    if (offsetFromBase >= 0) {
        qlonglong targetAddress = baseAddr + offsetFromBase;
        return QByteArray::number(targetAddress, 16);
    }

    return {};
}
