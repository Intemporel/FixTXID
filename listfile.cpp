#include "listfile.h"

Listfile::Listfile(QUrl listfile, QObject *parent) : QObject(parent)
{ url = listfile; }

Listfile::~Listfile() { }

void Listfile::initialize()
{
    connect(&WebClient, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(fileDownloaded(QNetworkReply*)));

    QNetworkRequest request(url);
    WebClient.get(request);
}

void Listfile::fileDownloaded(QNetworkReply *pReply)
{
    QFile f(QString("%1/listfile.csv").arg(QDir::currentPath()));
    if (f.open(QIODevice::WriteOnly))
    {
        f.write(pReply->readAll());
        f.flush();
        f.close();
    }

    pReply->deleteLater();
    emit downloaded();

    mapping();
}

void Listfile::mapping(QFile f)
{
    if (f.open(QIODevice::ReadOnly))
    {
        QTextStream in(&f);
        while(!in.atEnd())
        {
            QList<QString> values = in.readLine().split(';');

            if ( values[1].contains(".blp") )
                listfile[(qint32)values[0].toInt()] = values[1].replace(R"(/)", R"(\)");
        }
    }

    emit mapped();
}
