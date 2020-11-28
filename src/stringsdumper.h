#ifndef STRINGSDUMPER_H
#define STRINGSDUMPER_H

#include <QString>

class StringsDumper
{
public:
    static QVector< QVector<QString> > dumpStrings(QString file, const QVector<QString>& baseOffsets);
private:
    static QString getAddressFromOffset(QString offset, const QVector<QString>& baseOffsets);
};

#endif // STRINGSDUMPER_H
