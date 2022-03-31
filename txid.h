#ifndef TXID_H
#define TXID_H

#include "fix.h"
#include "listfile.h"

#include <QDir>
#include <QFile>
#include <QMimeData>
#include <QFileInfo>
#include <QMainWindow>
#include <QFileDialog>
#include <QDirIterator>
#include <QDragEnterEvent>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <QFileSystemModel>
#include <QModelIndexList>
#include <QModelIndex>

QT_BEGIN_NAMESPACE
namespace Ui { class TXID; }
QT_END_NAMESPACE

enum SendMessage {
    MSG_LOG,
    MSG_ERROR
};

enum keyMessage {
    NONE,
    PROGRESS,
    DONE,
    ERROR,
    KEY_COUNT
};

class TXID : public QMainWindow
{
    Q_OBJECT

public:
    TXID(QWidget *parent = nullptr);
    ~TXID();

    Listfile listfile;
    Fix model;

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

private:
    Ui::TXID *ui;
    QMap<qint32, QString> textures;
    QMap<QString, qint8> errorList;
    QFileSystemModel* file_model;

    int fixed = 0;
    int count = 0;
    int error = 0;
    int alreadyConverted = 0;
    QString dir;

    void generateModelList(QString);
    bool listfileExist();
    void populateFileAlreadyConverted(QString modelName);

    QString getError(qint8);
    void sendMessage(SendMessage, keyMessage, QString);
    QString getPrefix[KEY_COUNT] = {
        "<b>",
        "<b style=\"color:orange;\">Progress >> ",
        "<b style=\"color:green;\">Done >> ",
        "<b style=\"color:red;\">Error >> "
    };

private slots:
    // listfile.h
    void listfileDownloaded();
    void listfileMapped();
    // fix.h
    void updateCount();
    void updateError(QString, qint8);
    void updateProgressBar();
    // ui_txid.h
    void selectDirectory();
    void fixAllModel();
    void fixAllModelShadow();

    void updateList();
};
#endif // TXID_H
