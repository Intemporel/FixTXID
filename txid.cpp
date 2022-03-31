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

    connect(ui->selectDir,  &QPushButton::clicked,              [this]() { selectDirectory(); });
    connect(ui->removeSel,  &QPushButton::clicked,              [this]() { ui->listModel->clear(); });

    connect(ui->fix,        &QPushButton::clicked,              [this]() { fixAllModel(); });
    connect(ui->fix_shadow, &QPushButton::clicked,              [this]() { fixAllModelShadow(); });

    connect(ui->treeView, &QTreeView::clicked, [this]() { updateList(); });
    connect(ui->treeView, &QTreeView::expanded, [this]() { ui->treeView->resizeColumnToContents(0); });
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

void TXID::generateModelList(QString path)
{
    if (!(QFileInfo(path)).isDir())
        return;

    file_model = new QFileSystemModel(this);
    file_model->setRootPath(path + "/");
    file_model->setNameFilterDisables(false);

    QStringList filters;
    filters << "*.m2";
    filters << "*.mdx";

    file_model->setNameFilters(filters);

    ui->treeView->setModel(file_model);
    ui->treeView->setRootIndex(file_model->index(path + "/"));
    ui->treeView->resizeColumnToContents(0);

    for (int i = 1; i < file_model->columnCount(); ++i)
        ui->treeView->hideColumn(i);

    sendMessage(MSG_LOG, NONE, "Dropped directory from : <b>" + path + "</b>");
    updateCount();
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
    ui->listModel->clear();
}

void TXID::fixAllModelShadow()
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
            model.getInformation(true);
            model.updateBoneLookupTable();
        }
        catch (const std::exception &e)
        {
            sendMessage(MSG_ERROR, ERROR, e.what());
        }
    }

    sendMessage(MSG_LOG, DONE, "All Shadow fixed !");

    if (error > 0)
        sendMessage(MSG_LOG, ERROR, QString("Found %1 errors.").arg(error));

    if (alreadyConverted > 0)
        sendMessage(MSG_LOG, NONE, QString("%1/%2 models still already converted").arg(alreadyConverted).arg(count));

    fixed = 0; count = 0, error = 0, alreadyConverted = 0;
    ui->fileFixed->clear();
    ui->listModel->clear();
}

void TXID::updateList()
{
    ui->listModel->clear();
    QModelIndexList indexes = ui->treeView->selectionModel()->selectedIndexes();
    if (indexes.size() > 0)
    {
        for (int n = 0; n < indexes.size()/4; ++n)
        {
            auto name = indexes.at(n * 4).data(0).toString();

            if (name.contains(".m2") || name.contains(".mdx"))
            {
                QString file = file_model->filePath(indexes.at(n * 4));
                if(ui->listModel->findItems(file, Qt::MatchContains).isEmpty())
                    ui->listModel->addItem(file);
            }
            else
            {
                QDirIterator it(file_model->filePath(indexes.at(n * 4)), QStringList() << "*.m2", QDir::Files, QDirIterator::Subdirectories);

                while (it.hasNext())
                {
                    QString file = it.next();
                    if(ui->listModel->findItems(file, Qt::MatchContains).isEmpty())
                        ui->listModel->addItem(file);
                }
            }
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
