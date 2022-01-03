#include "error.h"

QString error::ERROR(qint8 value)
{
    switch (value) {
    case 0:
        return "file doesn't exist or not found.";
        break;
    case 1:
        return "model is already converted.";
        break;
    case 2:
        return "TXID chunk not found";
        break;
    default:
        return "Error not found.";
        break;
    }

    return "Error not found.";
}

void error::generateErrorLog()
{
    if (arr.isEmpty())
        return;

    QFile f(file);
    if (f.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&f);

        out << "FixTXID - EIntemporel \n";
        out << "-----\n";

        QMapIterator<QString, qint8> i(arr);
        while (i.hasNext())
        {
            i.next();
            out << "Error: " << i.key() << " >> " << ERROR(i.value()) << "\n";
        }
    }

    f.close();
}


