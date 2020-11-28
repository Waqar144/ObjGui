#ifndef SECTION_H
#define SECTION_H

#include "QString"
#include "QVector"

class Section
{
public:
    Section();
    Section(QString section, QVector< QVector<QByteArray> > contents);
    const QString& getSectionName() const;
    QByteArray getAddressAt(int index) const;
    int getMatrixLen() const;
    QVector<QByteArray> getLine(int line) const;
    QByteArray getHexString() const;
    QByteArray getAddressString() const;

private:
    QString sectionName;
    QVector< QVector<QByteArray> > sectionMatrix;
    int matrixLen;
};

#endif // SECTION_H
