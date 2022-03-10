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

    connect(&listfile, SIGNAL(mapped()), this, SLOT(listfileMapped()));
    connect(&listfile, SIGNAL(downloaded()), this, SLOT(listfileDownloaded()));

    connect(&model, SIGNAL(updated()), this, SLOT(updateProgressBar()));
    connect(&model, SIGNAL(sendError(QString, qint8)), this, SLOT(updateError(QString, qint8)));

    if (listfileExist())
    {
        sendMessage(MSG_LOG, NONE, "<b>We found listfile in the directory's application</b>");
        sendMessage(MSG_LOG, PROGRESS, "Mapping listfile");
        listfile.mapping();
    }
    else
    {
        sendMessage(MSG_LOG, NONE, "<b>We can't found listfile in the directory's application</b>");
        sendMessage(MSG_LOG, PROGRESS, "Downloading listfile");
        listfile.initialize();
    }

    connect(ui->selectDir, &QPushButton::clicked,              [this]() { selectDirectory(); });
    connect(ui->removeSel, &QPushButton::clicked,              [this]() { removeSelectedItem(); });
    connect(ui->fix,       &QPushButton::clicked,              [this]() { fixAllModel(); });
    connect(ui->treeModel, &QTreeWidget::itemSelectionChanged, [this]() { listModelUpdate(); });
    connect(ui->treeModel, &QTreeWidget::itemExpanded,         [this]() { ui->treeModel->resizeColumnToContents(0); });

    initializeTree();
}

TXID::~TXID() { delete ui; }

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

        dir = data.absoluteDir().absolutePath();
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

    sendMessage(MSG_LOG, NONE, "Dropped directory from : <b>" + path + "</b>");

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

void TXID::populateFileAlreadyConverted(QString modelName)
{
    QFile f("AlreadyConverted.csv");
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        QTextStream in(&f);
        in << modelName.remove(dir) << ",\n";
        f.close();
    }
}

void TXID::updateCount()
{
    count = ui->listModel->count();
    ui->progress->setMaximum(count);
    ui->fileFixed->setText(QString::number(fixed) + "/" + QString::number(count));
}

void TXID::updateError(QString model, qint8 errorType)
{
    error++;
    if (errorType == 1) //MD20Size <= 264
    {
        alreadyConverted++;

        if (ui->alreadyConnected->isChecked())
            populateFileAlreadyConverted(model);
    }
    else
    {
        sendMessage(MSG_ERROR, ERROR, model + " >> " + getError(errorType));
    }
    updateProgressBar();
}

void TXID::updateProgressBar()
{
    fixed++;
    ui->fileFixed->setText(QString::number(fixed) + "/" + QString::number(count));
    ui->progress->setValue(fixed);
}

bool TXID::listfileExist()
{
    QFile *check = new QFile(QString("%1/listfile.csv").arg(QDir::currentPath()));
    if (check->exists())
        return true;

    return false;
}

void TXID::listfileDownloaded()
{
    sendMessage(MSG_LOG, DONE, "listfile is downloaded");
    sendMessage(MSG_LOG, PROGRESS, "Mapping listfile");
}

void TXID::listfileMapped()
{
    sendMessage(MSG_LOG, DONE, "listfile is mapped");
    ui->fix->setEnabled(true);
    model.setDictionnary(listfile.getListfile());
}

void TXID::fixAllModel()
{
    errorList.clear();
    fixed = 0;
    error = 0;
    alreadyConverted = 0;

    for (int i = 0; i < ui->listModel->count(); ++i)
    {
        QListWidgetItem *item = ui->listModel->item(i);
        try
        {
            model.setModel(item->text());
            model.run();
        } catch (const std::exception &e) {
            sendMessage(MSG_ERROR, ERROR, e.what());
        }
    }

    sendMessage(MSG_LOG, DONE, "All files fixed !");

    if (error > 0)
        sendMessage(MSG_LOG, ERROR, QString("Found %1 errors.").arg(error));

    if (alreadyConverted > 0)
        sendMessage(MSG_LOG, NONE, QString("%1/%2 models still already converted").arg(alreadyConverted).arg(count));

    fixed = 0; count = 0, error = 0, alreadyConverted = 0;
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

QString TXID::getError(qint8 value)
{
    switch (value) {
    case 0:
        return "File doesn't exist or not found.";
        break;
    case 1:
        return "Model is already converted to 3.3.5 version.";
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

void TXID::sendMessage(SendMessage t, keyMessage key, QString s)
{
    QString append = getPrefix[key] + s + "</b>";
    switch (t) {
    default:
    case MSG_LOG:
        ui->logBrowser->setHtml(ui->logBrowser->toHtml() + append);
        break;
    case MSG_ERROR:
        ui->errorBrowser->setHtml(ui->errorBrowser->toHtml() + append);
        break;
    }
}
