#ifndef FUNCTION_H
#define FUNCTION_H

#include "QString"
#include "QVector"

class Function
{
public:
    Function();
    Function(QString functionName, QString addr, QString section, QString offset, QVector< QVector<QByteArray> > contents);
    int getMatrixLen() const;
    bool isEmpty() const;
    void setXrefData(int index, QString xrefData);
    const QString& getName() const;
    const QString& getAddress() const;
    const QString& getSection() const;
    const QString& getFileOffset() const;
    const QString& getAddressAt(int index) const;
    QVector<QByteArray> getLine(int line) const;
    QByteArray getContents() const;
private:
    QString name;
    QString address;
    QString section;
    QString fileOffset;
    QVector< QVector<QByteArray> > functionMatrix;
    int matrixLen;

};

#endif // FUNCTION_H
