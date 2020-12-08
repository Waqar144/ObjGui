#include "strings.h"

#include "QDebug"

Strings::Strings(){
    matrixLen = 0;
}

// Sets the strings matrix
void Strings::setStringsData(std::tuple<QVector<QByteArray>, QVector<QByteArray> > &&data) {
    strings = std::move(std::get<0>(data));
    addresses = std::move(std::get<1>(data));

    matrixLen = addrsLong.length();
}

// Returns the index of a target address
int Strings::getIndexByAddress(QByteArray targetAddress) {
    if (!addresses.isEmpty()) {
        // Binary search
        int upperLimit = matrixLen - 1;
        int lowerLimit = 0;
        int currentIndex = upperLimit / 2;

        while (lowerLimit < upperLimit && currentIndex != 0){
            const QByteArray& currentAddress = addresses.at(currentIndex);

            if (currentAddress == targetAddress){
                return currentIndex;
            } else if (targetAddress < currentAddress){
                upperLimit = currentIndex - 1;
                currentIndex = (upperLimit + lowerLimit) / 2;
            } else{
                lowerLimit = currentIndex + 1;
                currentIndex = (upperLimit + lowerLimit) / 2;
            }

        }
        // Final checks
        if (addresses.at(currentIndex) == targetAddress){
            return currentIndex;
        } else if (matrixLen > 1 && addresses.at(1) == targetAddress){
            return 1;
        }
    }
    // If not found return -1
    return -1;
}

// Returns address at given index
QString Strings::getAddressAt(int index){
    if(index >= 0 && index < matrixLen)
        return addresses.at(index);
    else
        return "";
}

QString Strings::getStringAt(int index){
    if(index >= 0 && index < matrixLen){
        QString str = strings.at(index);
        return str.replace('\n', "\\n");
    } else {
        return QLatin1String("");
    }
}

// Return strings formatted for display
QString Strings::getStrings(){
    QString ret = "";
    for (int i = 0; i < matrixLen; i++){
        QString str = strings.at(i);
        // Escape new line characters
        str.replace('\n', "\\n");
        ret.append(str + "\n");
    }
    return ret;
}

// Return addresses of strings formatted for display
QString Strings::getStringsAddresses(){
    QString ret = "";
    for (int i = 0; i < matrixLen; i++){
        ret.append(addresses.at(i) + '\n');
    }
    return ret;
}

