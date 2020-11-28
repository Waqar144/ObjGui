#include "function.h"

#include "QDebug"

/*
 * Function contents stored in a matrix where each row is a line of output.
 * Colomns contain {address, hex, optcode, arguments, xref data}
 * Example:
 *      [000010]    [0b 0a 3c]    [jmp]     [000050]                 []
 *      [000000]    [0b 0a 3c]    [lea]     [400981]                 ["HelloWorld"]
 *      [000020]    [12 c9 5d]    [call]    [00230b <printf@plt>]    []
 *      ...
 */

Function::Function(){
    matrixLen = 0;
}

Function::Function(QString functionName, QString addr, QString sect, QString offset, QVector< QVector<QByteArray> > contents)
{
    name = functionName;
    address = addr;
    section = sect;
    fileOffset = offset;
    functionMatrix = contents;
    matrixLen = functionMatrix.length();
}

void Function::setXrefData(int index, QString xrefData){
    functionMatrix[index][4] = (QChar('"') + xrefData + QChar('"')).toLocal8Bit();
}

// Return the line(row) from the matrix at given index/line number
QVector<QByteArray> Function::getLine(int line) const {
    if (line >= 0 && line < matrixLen)
        return functionMatrix.at(line);
    else {
        QVector<QByteArray> empty;
        return empty;
    }
}

// Return contents of function matrix formatted for display
QByteArray Function::getContents() const{
    QByteArray contents;
    for (int i = 0; i < matrixLen; i++){
        QVector<QByteArray> line = getLine(i);
        contents.append(line[0] + "  ");
        contents.append(line[1] + " ");
        contents.append(line[2] + " ");
        contents.append(line[3] + "    ");
        contents.append(line[4] + "\n");
    }

    return contents;
}

// Retrun address at given index
const QString &Function::getAddressAt(int index) const {
    if (index >= 0 && index < matrixLen) {
        return functionMatrix.at(index).at(0);
    } else {
        return QLatin1String("");
    }
}

bool Function::isEmpty() const {
    if (matrixLen > 0)
        return true;
    else
        return false;
}

int Function::getMatrixLen() const {
    return matrixLen;
}

const QString &Function::getName() const{
    return name;
}

const QString & Function::getAddress() const {
    return address;
}

const QString & Function::getSection() const {
    return section;
}

const QString & Function::getFileOffset() const {
    return fileOffset;
}
