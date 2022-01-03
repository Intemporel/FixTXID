#ifndef ERROR_H
#define ERROR_H

#include <QMap>
#include <QFile>
#include <QTextStream>

class error : public QFile
{
    Q_OBJECT

public:
    explicit error(QObject *parent = nullptr) : QFile(parent) {};
    ~error() {};
    void generateErrorLog();
    void setArrayError(QMap<QString, qint8> m) { arr = m; };

private:
    QString file = "error.log";
    QMap<QString, qint8> arr;
    QString ERROR(qint8);
};

#endif // ERROR_H
