#ifndef TXID_H
#define TXID_H

#include "fix.h"
#include "error.h"
#include "listfile.h"

#include <QDir>
#include <QMimeData>
#include <QFileInfo>
#include <QMainWindow>
#include <QFileDialog>
#include <QDirIterator>
#include <QDragEnterEvent>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QT_BEGIN_NAMESPACE
namespace Ui { class TXID; }
QT_END_NAMESPACE

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

    int fixed = 0;
    int count = 0;

    void generateModelList(QString);
    bool listfileExist();
    void initializeTree();

private slots:
    // listfile.h
    void loadListfile();
    void generatedMap();
    // fix.h
    void updateCount();
    void updateError(QString, qint8);
    void updateProgressBar();
    // ui_txid.h
    void selectDirectory();
    void removeSelectedItem(bool onlyListModel = false);
    void fixAllModel();
    void listModelUpdate();
};
#endif // TXID_H
