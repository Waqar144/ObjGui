#include "section.h"

/*
 * Section contents stored in a matrix where each row is a line of output.
 * Colomns contain {address, hex}
 * Example:
 *      [000000]    [0b 0a 3c]
 *      [000010]    [0b 0a 3c]
 *      [000020]    [12 c9 5d]
 *      ...
 */

Section::Section()
{

}

Section::Section(QString section, QVector< QVector<QByteArray> > contents){
    sectionName = section;
    sectionMatrix = contents;
    matrixLen = sectionMatrix.length();
}

// Return address at given index
QByteArray Section::getAddressAt(int index) const {
    if (index >= 0 && index < matrixLen)
        return sectionMatrix.at(index).at(0);
    else
        return "";
}

// Return the line(row) from the matrix at the given index/line number
QVector<QByteArray> Section::getLine(int line)const {
    if (line > 0 && line < matrixLen)
        return sectionMatrix.at(line);
    else {
        QVector<QByteArray> empty;
        return empty;
    }
}

// Get a string of all hex values seperated by line breaks
QByteArray Section::getHexString()const {
    QByteArray hexStr;
    for (int i = 0; i < matrixLen; i++){
        hexStr.append(sectionMatrix.at(i)[1] + "\n");
    }

    return hexStr;
}

// Get a string of all address values seperated by line breaks
QByteArray Section::getAddressString()const {
    QByteArray addrStr = "";
    for (int i = 0; i < matrixLen; i++){
        addrStr.append(sectionMatrix.at(i)[0] + "\n");
    }

    return addrStr;
}

const QString& Section::getSectionName() const {
    return sectionName;
}

int Section::getMatrixLen()const {
    return matrixLen;
}
