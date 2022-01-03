#include "txid.h"
#include "ui_txid.h"

TXID::TXID(QWidget *parent)
    : QMainWindow(parent)
    , listfile(QUrl("https://wow.tools/casc/listfile/download/csv/unverified/"), this)
    , model(this)
    , ui(new Ui::TXID)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->fix->setEnabled(false);

    connect(&listfile, SIGNAL(mapped()), this, SLOT(generatedMap()));
    connect(&listfile, SIGNAL(downloaded()), this, SLOT(loadListfile()));

    connect(&model, SIGNAL(updated()), this, SLOT(updateProgressBar()));
    connect(&model, SIGNAL(sendError(QString, qint8)), this, SLOT(updateError(QString, qint8)));

    if (listfileExist())
    {
        ui->listfileProgress->setText("Progress: Mapping listfile.csv");
        listfile.mapping();
    }
    else
    {
        ui->listfileProgress->setText("Progress: Downloading listfile.csv ...");
        listfile.initialize();
    }

    connect(ui->selectDir, &QPushButton::clicked, [=]() {selectDirectory();});
    connect(ui->removeSel, &QPushButton::clicked, [=]() {removeSelectedItem();});
    connect(ui->fix, &QPushButton::clicked, [=]() {fixAllModel();});
    connect(ui->treeModel, &QTreeWidget::itemSelectionChanged, [=]() {listModelUpdate();});
    connect(ui->treeModel, &QTreeWidget::itemExpanded, [=]() {ui->treeModel->resizeColumnToContents(0);});

    initializeTree();
}

TXID::~TXID()
{
    delete ui;
}

void TXID::dragEnterEvent(QDragEnterEvent *e)
{
    if ( e->mimeData()->hasUrls() )
        e->acceptProposedAction();
}

void TXID::dropEvent(QDropEvent *e)
{
    foreach ( const QUrl &url, e->mimeData()->urls())
    {
        const QFileInfo data(url.toLocalFile());

        // only file
        if ( data.suffix() == "m2" )
            ui->listModel->addItem(data.absoluteFilePath());

        if ( data.isDir() )
            generateModelList(data.absoluteFilePath());
    }

    updateCount();
}

void TXID::selectDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                "/",
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
        generateModelList(dir);
}

void TXID::removeSelectedItem(bool onlyListModel)
{
    ui->listModel->clear();

    if ( onlyListModel )
        return;

    for (QTreeWidgetItem* item : ui->treeModel->selectedItems())
    {
        QTreeWidgetItem* parent = item->parent();

        if (parent)
            parent->removeChild(item);

        delete item;
    }
}

void TXID::generateModelList(QString path)
{
    if (!(QFileInfo(path)).isDir())
        return;

    QDirIterator dit(path, QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::Subdirectories);

    // Initialize folder
    QRegularExpression pattern("^(.*)/");
    QRegularExpressionMatch match = pattern.match(path);
    QString start = path.mid(match.capturedEnd(), path.length() - match.capturedEnd());
    QRegularExpression reg("^(.*)" + start + "/");
    qint8 length = start.size() + 1;

    QTreeWidgetItem *root = new QTreeWidgetItem(ui->treeModel);
    root->setText(0, start);
    root->setText(1, path);
    ui->treeModel->addTopLevelItem(root);

    while (dit.hasNext())
    {
        QString saved = dit.next();
        QRegularExpressionMatch temp = reg.match(saved);
        QString current = saved.mid(temp.capturedEnd() - length, path.length() - temp.capturedEnd() - length);
        bool isChild = false;

        QTreeWidgetItem *item = new QTreeWidgetItem();
        QString text0 = current.mid(length, current.length() - length);
        item->setText(0, text0);
        item->setText(1, saved);

        for (int i = 0; i < ui->treeModel->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem *twi = ui->treeModel->topLevelItem(i);
            if ( current.contains(twi->text(0)) )
            {
                qint8 tempLen;
                for (int a = 0; a < twi->childCount(); ++a)
                {
                    QTreeWidgetItem *itemA = twi->child(a);
                    if ( current.contains(itemA->text(0) + R"(/)") )
                    {
                        tempLen = itemA->text(0).length() + 1;
                        text0 = text0.mid(tempLen, text0.length() - tempLen);
                        item->setText(0, text0);

                        for (int b = 0; b < itemA->childCount(); b++)
                        {
                            QTreeWidgetItem *itemB = itemA->child(b);
                            if ( current.contains(itemB->text(0) + R"(/)") )
                            {
                                tempLen = itemB->text(0).length() + 1;
                                text0 = text0.mid(tempLen, text0.length() - tempLen);
                                item->setText(0, text0);

                                for (int c = 0; c < itemB->childCount(); ++c)
                                {
                                    QTreeWidgetItem *itemC = itemB->child(c);
                                    if ( current.contains(itemC->text(0) + R"(/)") )
                                    {
                                        tempLen = itemC->text(0).length() + 1;
                                        text0 = text0.mid(tempLen, text0.length() - tempLen);
                                        item->setText(0, text0);

                                        for (int d = 0; d < itemC->childCount(); ++d)
                                        {
                                            QTreeWidgetItem* itemD = itemC->child(d);
                                            if ( current.contains(itemD->text(0) + R"(/)") )
                                            {
                                                tempLen = itemD->text(0).length() + 1;
                                                text0 = text0.mid(tempLen, text0.length() - tempLen);
                                                item->setText(0, text0);

                                                itemD->addChild(item);
                                                isChild = true;
                                                break;
                                            }
                                        }

                                        if ( isChild )
                                            break;

                                        itemC->addChild(item);
                                        isChild = true;
                                        break;
                                    }
                                }

                                if ( isChild )
                                    break;

                                itemB->addChild(item);
                                isChild = true;
                                break;
                            }
                        }

                        if ( isChild )
                            break;

                        itemA->addChild(item);
                        isChild = true;
                        break;
                    }
                }


                if ( !isChild )
                {
                    twi->addChild(item);
                    break;
                }
            }
        }
    }

    ui->treeModel->resizeColumnToContents(0);
    updateCount();
}

void TXID::initializeTree()
{
    ui->treeModel->setColumnCount(2);

    QStringList labels;
    labels << "Directory" << "Path";
    ui->treeModel->setHeaderLabels(labels);
}

void TXID::updateCount()
{
    count = ui->listModel->count();
    ui->progress->setMaximum(count);
    ui->fileFixed->setText(QString("%1/%2").arg(fixed).arg(count));
}

void TXID::updateError(QString model, qint8 errorType)
{
    errorList[model] = errorType;
    ui->error->setText(QString("Error count : %1").arg(errorList.count()));
}

void TXID::updateProgressBar()
{
    fixed++;
    ui->fileFixed->setText(QString("%1/%2").arg(fixed).arg(count));
    ui->progress->setValue(fixed);
}

bool TXID::listfileExist()
{
    QFile *check = new QFile(QString("%1/listfile.csv").arg(QDir::currentPath()));
    if (check->exists())
        return true;

    return false;
}

void TXID::loadListfile()
{
    ui->listfileProgress->setText("Done: listfile.csv generated");
    ui->fix->setEnabled(true);
}

void TXID::generatedMap()
{
    ui->listfileProgress->setText("Done: listfile.csv generated");
    ui->fix->setEnabled(true);

    model.setDictionnary(listfile.getListfile());
}

void TXID::fixAllModel()
{
    errorList.clear();
    fixed = 0;

    for (int i = 0; i < ui->listModel->count(); ++i)
    {
        QListWidgetItem *item = ui->listModel->item(i);

        try
        {
            model.setModel(item->text());
            model.run();

            ui->fileName->setText(item->text());
        } catch (_exception e) {

        }
    }

    error err;
    err.setArrayError(errorList);
    err.generateErrorLog();

    fixed = 0; count = 0;
    ui->fileName->setText("All files fixed !");
    ui->fileFixed->clear();
    removeSelectedItem();
}

void TXID::listModelUpdate()
{
    removeSelectedItem(true);

    for (QTreeWidgetItem *item : ui->treeModel->selectedItems())
    {
        QString path = item->text(1);
        QDirIterator it(path, QStringList() << "*.m2", QDir::Files, QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            QString file = it.next();
            if(ui->listModel->findItems(file, Qt::MatchExactly).isEmpty())
                ui->listModel->addItem(file);
        }
    }

    updateCount();
}
