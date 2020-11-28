#include "objdumper.h"
#include "QVector"
#include "QStringRef"
#include "QProcess"

#include "QDebug"

ObjDumper::ObjDumper()
{
    // Set default options
    useCustomBinary = false;
    objdumpBinary = QStringLiteral("objdump");
    outputSyntax = QStringLiteral("intel");
    disassemblyFlag = QStringLiteral("-d");
    demangleFlag = QLatin1String("");
    target = QLatin1String("");
    insnwidth = 10;

    addressRegex.setPattern(QStringLiteral("[0-9a-f]+"));
    hexBytesRegex.setPattern(QStringLiteral("[0-9a-f ]+"));
}

// Runs objdump given arguments and returns outout
QByteArray ObjDumper::getDump(QStringList argsList){
    QString objdumpStr;

    if (useCustomBinary && objdumpBinary != "")
        objdumpStr = objdumpBinary;
    else
        objdumpStr = QStringLiteral("objdump");

    QProcess *process = new QProcess(0);
    process->start(objdumpStr, argsList);

    if (!process->waitForStarted())
        return ("");

    if (!process->waitForFinished())
        return ("");

    QByteArray output;
    output.append(process->readAllStandardError());
    output.append(process->readAllStandardOutput());

    return output;
}

// Parses disassembly and populates function list
QVector<Function> ObjDumper::getFunctionData(QString file, QVector<QString> baseOffsets){
   QVector<Function> functionList;
   const QString dump = getDisassembly(file);

    // Split dump into vector of string references to each function
    const QVector<QStringRef> dumpList = dump.splitRef(QStringLiteral("\n\n"));
    QString currentSection = QLatin1String("");

    // Parse dumplist
    for (int listIndex = 0; listIndex < dumpList.length(); listIndex++){
        const QStringRef& dumpStr = dumpList.at(listIndex);

        // Parse first word
        auto pos = dumpStr.indexOf(QLatin1Char(' '));
        QString tmp = dumpStr.left(pos).toString();
        int i = pos == -1 ? dumpStr.length() - 1 : pos;

        // Check if section or function
        if (tmp == QStringLiteral("Disassembly")){
            currentSection = dumpStr.mid(22).toString();

        } else if (tmp.startsWith(QLatin1Char('0')) /*tmp is address*/){
            // Get function address
            QString address = std::move(tmp);
            QString fileOffest = QString();
            QVector< QVector<QByteArray> > functionMatrix;

            // Get file offset
            fileOffest = getFileOffset(address, baseOffsets).at(0);

            // Get function name
            i += 2;
            int endlinePos = dumpStr.indexOf(QLatin1Char('\n'), i);
            QString name = dumpStr.mid(i, endlinePos - i - 2).toString();

            // Parse function contents
            const QStringRef contents = dumpStr.mid(i);
            const QVector<QStringRef> lines = contents.split(QLatin1Char('\n'));

            for (int lineNum = 0; lineNum < lines.length()-1; lineNum++){
                const QStringRef& line = lines.at(lineNum);
                QVector<QByteArray> row = parseFunctionLine(line);

                if (!row[0].isEmpty()){
                    functionMatrix.append(std::move(row));
                }

            }

            // Add to functionList
            Function function(std::move(name),
                              std::move(address),
                              std::move(currentSection),
                              std::move(fileOffest),
                              std::move(functionMatrix));
            functionList.push_back(std::move(function));
        }


    }
    return functionList;

}

QVector<QByteArray> ObjDumper::parseFunctionLine(const QStringRef& line){
    QVector<QByteArray> row(5);
    if (line.length() > insnwidth * 3) {
        int pos = 0;

        // Get address
        QByteArray address;
        address.reserve(16);
        while (pos < line.length() && line.at(pos) != QChar(':')){
            address.append(line.at(pos).toLatin1());
            pos++;
        }
        pos++;

        row[0] = parseAddress(address);

        // Skip whitespace
        while (pos < line.length() && (line.at(pos) == QChar(' ') || line.at(pos) == QChar('\t') )){
            pos++;
        }

        // Get hex
        QByteArray hexBytes = line.mid(pos, insnwidth * 3).toLocal8Bit();
        row[1] = parseHexBytes(hexBytes);

        pos += insnwidth * 3;

        // Skip whitespace
        while (pos < line.length() && (line.at(pos) == QChar(' ') || line.at(pos) == QChar('\t') )){
            pos++;
        }

        // Get optcode
        QByteArray opt;
        int x = line.length() - pos;
        opt.reserve(x > 0 ? x : 1);
        while (pos < line.length() && line.at(pos) != QChar(' ')){
            opt.append(line.at(pos).toLatin1());
            pos++;
        }
        pos++;

        row[2] = std::move(opt);

        while (pos < line.length() && line.at(pos) == QChar(' ')){
            pos++;
        }

        // Get args
        row[3] = line.mid(pos).toLocal8Bit();

        // empty by default, used for xref data
        row[4] = QByteArrayLiteral("");

        // Remove extra space from byte array
    } else {
        row[0] = "";
        row[1] = "";
        row[2] = "";
        row[3] = "";
        row[4] = "";
    }

    return row;
}

QByteArray ObjDumper::parseAddress(const QByteArray &address) {
    QByteArray ret = QByteArrayLiteral("0x");
    ret.reserve(2 + 4);
    for (int i = 0; i < address.length(); ++i) {
        if ( isspace(address.at(i)) )
            continue;

        bool addr = true;
        int j = i;
        while (j < address.length()) {
            if ( !isdigit(address.at(j)) ) {
                char c = address.at(j);
                bool hex = c >= 97 && c <= 102;
                if (!hex) {
                    addr = false;
                    break;
                }
            }
            j++;
        }

        if (addr) {
            for (int ii = i; ii < (j - i); ii++) {
                ret.append(address.at(i));
            }
            return ret;
        }
    }
    return QByteArrayLiteral("");
}

QByteArray ObjDumper::parseHexBytes(const QByteArray& byteString) {

    QByteArray ret;
    ret.reserve(byteString.size());
    for (int i = 0; i < byteString.length(); ++i) {
        if (byteString.at(i) == ' ')
            continue;
        char c = byteString.at(i);
        if (isdigit(c) || (c >= 97 && c <= 102)) {
            ret.append(c);
            continue;
        } else {
            return QByteArrayLiteral("");
        }
    }
    //insert padding
    int paddingLength = (insnwidth * 2) - ret.length();
    QByteArray padding;
    padding.fill(' ', paddingLength);
    ret.append(padding);
    return ret;
}

// Parses result of all contents(objdump -s) and populates section list
QVector<Section> ObjDumper::getSectionData(QString file){
    QVector<Section> sectionList;
    QString contents = getContents(file);

    QVector<QStringRef> contentsList = contents.splitRef("Contents of section ");

    // Parse contents list
    for (int listIndex = 0; listIndex < contentsList.length(); listIndex++){
        QString sectionName;
        QVector< QVector<QByteArray> > sectionMatrix;

        QStringRef contentsStr = contentsList.at(listIndex);

        // Get section name
        int i = 0;
        while (i < contentsStr.length()-1 && contentsStr.at(i) != QChar(':')){
            sectionName.append(contentsStr.at(i));
            i++;
        }

        QStringRef sectionContents = contentsStr.mid(i+2);

        // Split content into lines
        QVector<QStringRef> lines = sectionContents.split("\n");

        // Parse each line and add data to lists
        for (int lineNum = 0; lineNum < lines.length()-1; lineNum++){
            QStringRef line = lines.at(lineNum);
            QVector<QByteArray> row = parseSectionLine(line);

            sectionMatrix.append(row);
        }

        // Insert new section
        Section section(sectionName, sectionMatrix);
        sectionList.push_back(section);

    }

    return sectionList;
}

QVector<QByteArray> ObjDumper::parseSectionLine(QStringRef line){
    QVector<QByteArray> row(2);

    // Get Address
    QByteArray address;
    address.reserve(4);
    int pos = 1;
    while (pos < line.length() && line.at(pos) != QChar(' ')){
        address.append(line.at(pos).toLatin1());
        pos++;
    }
    row[0] = address;

    pos++;

    // Next 35 chars are hex followed by 2 spaces
    QByteArray hexStr = line.mid(pos, 35).toLocal8Bit();

    // Add space between each byte(default is space between 4 byte words)
    for (int i = 2; i < hexStr.length(); i+=3){
        if (hexStr.at(i) != QChar(' ')){
            hexStr.insert(i, ' ');
        }
    }

    row[1] = hexStr;

    // Ignore ascii

    // Remove extra space from byte array
//    row[0].squeeze();
//    row[1].squeeze();

    return row;
}

// Get file format by parsing header
QString ObjDumper::getFileFormat(QString file){
    QStringList argsList;
    if (!target.isEmpty())
        argsList << target;
    argsList << "-f" << file;
    QString header = getDump(argsList);
    QString fileFormat = "";

    if (!header.contains("File format not recognized")){
        // Extract file format from header
        int i = 0;
        int newlineCount = 0;
        while (i < header.length() && newlineCount < 2){
            if (header.at(i) == QChar('\n'))
                newlineCount++;
            i++;
        }

        i-=2;
        while (i >= 0 && header.at(i) != QChar(' ')) {
            fileFormat.prepend(header.at(i));
            i--;
        }
    }

    return fileFormat;

}

// Get disassembly: objdump -d
QByteArray ObjDumper::getDisassembly(QString file){
    QStringList argsList;
    if (!target.isEmpty())
        argsList << target;
    argsList << "--insn-width=" + QString::number(insnwidth) << demangleFlag << "-M" << outputSyntax << disassemblyFlag << file;
    QByteArray disassembly = getDump(argsList);
    return removeHeading(disassembly, 4);
}

// Get symbols table: objdump -t
QString ObjDumper::getSymbolsTable(QString file){
    QStringList argsList;
    if (!target.isEmpty())
        argsList << target;
    argsList << demangleFlag << "-t" << file;
    QByteArray symbolsTable = getDump(argsList);
    return removeHeading(symbolsTable, 4);
}

// Get relocation entries: objdump -R
QString ObjDumper::getRelocationEntries(QString file){
    QStringList argsList;
    if (!target.isEmpty())
        argsList << target;
    argsList << demangleFlag << "-R" << file;
    QByteArray relocationEntries = getDump(argsList);
    return removeHeading(relocationEntries, 4);
}

// Get all contents(hexdump of sections): objdump -s
QString ObjDumper::getContents(QString file){
    QStringList argsList;
    if (!target.isEmpty())
        argsList << target;
    argsList << "-s" << file;
    QByteArray contents = getDump(argsList);
    return removeHeading(contents, 3);
}

// Get archive, file and private headers: objdump -a -f -p
QString ObjDumper::getHeaders(QString file){
    QStringList argsList;
    if (!target.isEmpty())
        argsList << target;
    argsList << "-a" << "-f" << "-p" << file;
    QByteArray headers = getDump(argsList);
    return removeHeading(headers, 3);
}

// Try to dump a header with objdump and return any errors
QString ObjDumper::checkForErrors(QString file){
    QStringList argsList;
    if (!target.isEmpty())
        argsList << target;
    argsList << "-f" << file;
    QString dumpStr = getDump(argsList);
    return parseDumpForErrors(dumpStr);
}

// Check for objdumps output errors. returns empty string if no errors found
QString ObjDumper::parseDumpForErrors(QString dump){
    if (dump.contains("UNKNOWN")){
        return "Architecture unknown.";
    } else if (dump.contains("File format not recognized")){
        return "Format not recognized.";
    }else if (dump.contains("File format is ambiguous")){
        QVector<QStringRef> dumpList = dump.splitRef(":");
        if (dumpList.length() == 5){
            // returns "Matching formats:[format1] [format2] [format3]..."
            return dumpList.at(3).toString() + ":" + dumpList.at(4).toString();
        }
    }

    return "";
}

// Returns base offset [base vma, base file offset]
QVector<QString> ObjDumper::getBaseOffset(QString file){
    QVector<QString> baseOffset(2);
    QStringList argsList;

    if (!target.isEmpty())
        argsList << target;

    argsList << "-h" << file;
    QString sectionHeader = getDump(argsList);

    if (!sectionHeader.isEmpty() && !sectionHeader.contains("File format not recognized")){
        QVector<QStringRef> headerLines = sectionHeader.splitRef('\n');

        if (headerLines.size() > 5) {
            QStringRef firstSection = headerLines.at(5);

            QVector<QStringRef> sectionVector = firstSection.split(' ', QString::SkipEmptyParts);

            if (sectionVector.size() > 5){
                QString baseAddress = sectionVector.at(3).toString();
                QString baseFileOffset = sectionVector.at(5).toString();

                QRegularExpressionMatch addressMatch = addressRegex.match(baseAddress);
                QRegularExpressionMatch fileOffsetMatch = addressRegex.match(baseFileOffset);

                if (addressMatch.hasMatch() && fileOffsetMatch.hasMatch()){
                    baseOffset[0] = baseAddress;
                    baseOffset[1] = baseFileOffset;

                    return baseOffset;
                }
            }
        }
    }

    baseOffset[0] = "000000";
    baseOffset[1] = "000000";

    return baseOffset;
}

// Get file offset of virtual memory address as vecotor [hex value, decimal value]
QVector<QString> ObjDumper::getFileOffset(QString targetAddress, QVector<QString> baseOffsets){
    QVector<QString> fileOffset(2);
    fileOffset[0] = QLatin1String("");
    fileOffset[1] = QLatin1String("");
    bool targetAddrOk;
    bool baseAddrOk;
    bool baseOffsetOk;
    qlonglong targetAddr = targetAddress.toLongLong(&targetAddrOk, 16);
    qlonglong baseAddr = baseOffsets[0].toLongLong(&baseAddrOk, 16);
    qlonglong baseOffset = baseOffsets[1].toLongLong(&baseOffsetOk, 16);

    if (targetAddrOk && baseAddrOk && baseOffsetOk){
        if (targetAddr >= baseAddr){
            qlonglong targetOffset = (targetAddr - baseAddr) + baseOffset;
            fileOffset[0] = QStringLiteral("0x") + QString::number(targetOffset, 16);
            fileOffset[1] = QString::number(targetOffset);
        }
    }

    return fileOffset;
}

// Removes heading(first [numLines] lines of objdump output)
QByteArray ObjDumper::removeHeading(QByteArray dump, int numLines){
    int i = 0;
    int newlineCount = 0;
    while (i < dump.length() && newlineCount < numLines){
        if (dump.at(i) == QChar('\n'))
            newlineCount++;
        i++;
    }

    return dump.mid(i);
}

// Returns first [numLines] of given string
QString ObjDumper::getHeading(QString dump, int numLines){
    int i = 0;
    int newlineCount = 0;
    while (i < dump.length() && newlineCount < numLines){
        if (dump.at(i) == QChar('\n'))
            newlineCount++;
        i++;
    }

    return dump.left(i);
}

//  Set Options
void ObjDumper::setUseCustomBinary(bool useCustom){
    useCustomBinary = useCustom;
}

void ObjDumper::setobjdumpBinary(QString binary){
    objdumpBinary = binary;
}

void ObjDumper::setOutputSyntax(QString syntax){
    outputSyntax = syntax;
}

void ObjDumper::setDisassemblyFlag(QString flag){
    disassemblyFlag = flag;
}

void ObjDumper::setDemangleFlag(QString flags){
    demangleFlag = flags;
}

// Set target flag: "-b [target]"
void ObjDumper::setTarget(QString trgt){
    target = trgt;
}
