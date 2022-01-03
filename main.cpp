#include "txid.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TXID w;
    w.show();
    return a.exec();
}
