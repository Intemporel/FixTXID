#ifndef LISTFILE_H
#define LISTFILE_H

#include <QMap>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

class Listfile : public QObject
{
    Q_OBJECT

public:
    Listfile(QUrl, QObject*);
    virtual ~Listfile();
    void initialize();
    void mapping(QFile f = QFile((QString("%1/listfile.csv").arg(QDir::currentPath()))));
    QMap<qint32, QString> getListfile() { return listfile; };

signals:
    void downloaded();
    void mapped();

private slots:
    void fileDownloaded(QNetworkReply*);

private:
    QUrl url;
    QNetworkAccessManager WebClient;
    QMap<qint32, QString> listfile;
};

#endif // LISTFILE_H
