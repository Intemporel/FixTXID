#ifndef FIX_H
#define FIX_H

//#include <QObject>
#include <QMap>
#include <QFile>
#include <QThread>
#include <QByteArray>
#include <QDataStream>

class Fix : public QThread
{
    Q_OBJECT

public:
    Fix(QObject *parent) : QThread(parent) {};
    virtual ~Fix() { /*delete this;*/ };
    void run();
    void setModel(QString m) {
        model = m;
        txid.clear();
        texture.clear();
    };
    void setDictionnary(QMap<qint32, QString> dic) {Dictonnary = dic;};
    bool getInformation(bool onlyShadow = false);
    void updateTextures();
    void updateBoneLookupTable();

signals:
    void updated();
    void sendError(QString, qint8);

private:
    QString model;
    QMap<qint32, QString> Dictonnary;
    qint8 error_type = -1;

    qint32 MD20Size;

    qint32 nTextures;
    qint32 offTextures;

    qint32 nBoneLookupTable;
    qint32 ofsBoneLookupTable;

    QVector<int> txid;
    QVector<QString> texture;
};

#endif // FIX_H
