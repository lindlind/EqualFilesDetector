#include "mainwindow.h"
#include "ui_mainwindow.h"

std::map <QByteArray, std::vector <QFile*> > MainWindow::filesByHash;
QFutureWatcher<void> MainWindow::watcher, MainWindow::preparingWatcher;
std::vector <std::pair <std::vector <QFile*>, qint64> > MainWindow::threadFiles;
std::vector <QFile*> MainWindow::allFiles;
QMutex MainWindow::mutex;
qint64 MainWindow::checkedFilesSize, MainWindow::sumSize;
int MainWindow::threadsNumber, MainWindow::shift;
bool MainWindow::threadsWasCancelled;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new
Ui::MainWindow)
{
    ui->setupUi(this);
    threadsNumber = QThread::idealThreadCount();
    threadsWasCancelled = false;
    connect(&preparingWatcher, SIGNAL(finished()), this, SLOT(search()));
    connect(this, SIGNAL(openFinalWindow()), &finalWindow, SLOT(showWindow()));
    connect(&finalWindow, SIGNAL(closeWindow()), this, SLOT(closeFinalWindow()));
}

MainWindow::~MainWindow()
{
    delete ui;
    finalWindow.close();
}

void MainWindow:: setFlag() {
    threadsWasCancelled = true;
}

void MainWindow::findEqual(std::pair <std::vector <QFile*>, qint64> &pairs) {
    std::vector <QFile*> &files = pairs.first;
    qint64 threadSize = pairs.second;
    std::map <QByteArray, std::vector <QFile*> > localFilesByHash;
    int smallFilesSizeDelta = 0;
    QCryptographicHash hash(QCryptographicHash::Sha512);

    for (int j = 0; j < (int)files.size(); j++) {
        hash.reset();
        if (threadsWasCancelled) {
            return;
        }

        if (files[j]->size() < (threadSize>>9)) {

            files[j]->open(QFile::ReadOnly);
            while (!files[j]->atEnd()) {
                hash.addData(files[j]->read(1<<17));
                if (threadsWasCancelled) {
                    files[j]->close();
                    return;
                }
            }
            smallFilesSizeDelta += files[j]->size();
            files[j]->close();


        } else {

            files[j]->open(QFile::ReadOnly);
            qint64 gotSize;
            gotSize = files[j]->pos();
            while (!files[j]->atEnd()) {
                hash.addData(files[j]->read(1<<17));
                if (threadsWasCancelled) {
                    files[j]->close();
                    return;
                }
                if (files[j]->pos() - gotSize >= (threadSize>>9)) {
                    mutex.lock();
                    checkedFilesSize += files[j]->pos() - gotSize;
                    gotSize = files[j]->pos();
                    emit watcher.progressValueChanged(checkedFilesSize>>shift);
                    mutex.unlock();
                }
            }
            smallFilesSizeDelta += files[j]->pos() - gotSize;
            files[j]->close();
        }

        localFilesByHash[hash.result()].push_back(files[j]);

        if (smallFilesSizeDelta >= (threadSize>>9)) {
            mutex.lock();
            checkedFilesSize += smallFilesSizeDelta;
            smallFilesSizeDelta = 0;
            emit watcher.progressValueChanged(checkedFilesSize>>shift);
            mutex.unlock();
        }

    }

    if (threadsWasCancelled) {
        return;
    }

    mutex.lock();
    checkedFilesSize += smallFilesSizeDelta;
    emit watcher.progressValueChanged(checkedFilesSize>>shift);

    for (auto& cur : localFilesByHash) {
        std::copy(cur.second.begin(), cur.second.end(), std::back_inserter(filesByHash[cur.first]));
    }
    mutex.unlock();
}

void MainWindow::getAllFiles(QString dir) {
    QDir directory(dir);
    QStringList files = directory.entryList(QDir::Files | QDir::NoSymLinks, QDir::NoSort);
    foreach (auto file, files) {
        allFiles.push_back(new QFile(dir + "/" + file));
    }

    QStringList dirs = directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::NoSort);
    foreach (QString currentDirectory, dirs) {
        getAllFiles(dir + "/" + currentDirectory);
    }
}

void MainWindow::deleteUniqueFiles() {
    std::sort(allFiles.begin(), allFiles.end(), [] (QFile* const &a, QFile* const &b) { return (a->size() > b->size()); });

    std::vector <QFile*> allFilesWithoutUnique;
    for (int i = 0; i < (int)allFiles.size(); i++) {
        if (allFiles[i]->open(QFile::ReadOnly)) {
            allFiles[i]->close();
            allFilesWithoutUnique.push_back(allFiles[i]);
        }
    }
    allFiles.swap(allFilesWithoutUnique);
    allFilesWithoutUnique.clear();

    if (allFiles.size() > 1 && allFiles[0]->size() == allFiles[1]->size()) {
        allFilesWithoutUnique.push_back(allFiles[0]);
    }
    for (int i = 1; i < (int)allFiles.size() - 1; i++) {
        if (allFiles[i-1]->size() == allFiles[i]->size() || allFiles[i+1]->size() == allFiles[i]->size()) {
            allFilesWithoutUnique.push_back(allFiles[i]);
        }
    }
    if (allFiles.size() > 1 && allFiles[allFiles.size() - 1]->size() == allFiles[allFiles.size() - 2]->size()) {
        allFilesWithoutUnique.push_back(allFiles.back());
    }

    allFiles.swap(allFilesWithoutUnique);
}

void MainWindow::on_checkBox_Threads_clicked() {
    if (ui->checkBox_Threads->isChecked()) {
        threadsNumber = QThread::idealThreadCount();
        ui->statusBar->showMessage("Установлено потоков: " + QString::number(threadsNumber) + ".");
    } else {
        threadsNumber = 1;
        ui->statusBar->showMessage("Программа будет использовать 1 поток.");
    }
}

void MainWindow::on_pushButton_Browse_clicked() {
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    "Select Directory for Scanning",
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->lineEdit_Directory->setText(dir);
    ui->statusBar->showMessage("Директория выбрана.");
}

void MainWindow::on_pushButton_Start_clicked() {

    const QFileInfo directory(ui->lineEdit_Directory->text());
    if (!directory.exists() || !directory.isDir()) {
        ui->statusBar->showMessage("Неверный путь.");
        QMessageBox::warning(this, "Ошибка!", "Неправильно указана путь до папки.\nПроверьте правильность и измените путь,\nили воспользуйтесь кнопкой \"Выбрать...\"");
        return;
    }
    if (directory.isSymLink()) {
        ui->statusBar->showMessage("Неверный путь.");
        QMessageBox::warning(this, "Ошибка!", "Папка является ярлыком.\nИзмените путь или воспользуйтесь кнопкой \"Выбрать...\"");
        return;
    }

    ui->statusBar->showMessage("Идет проверка директории.");
    ui->checkBox_Threads->setEnabled(false);
    ui->pushButton_Browse->setEnabled(false);
    ui->pushButton_Start->setEnabled(false);
    ui->lineEdit_Directory->setEnabled(false);
    preparingWatcher.setFuture(QtConcurrent::run(&MainWindow::preparing, ui->lineEdit_Directory->text()));
}

void MainWindow::preparing(QString dir) {
    getAllFiles(dir);
    if (allFiles.size() == 0) {
        return;
    }
    deleteUniqueFiles();
    sumSize = 0;
    threadFiles.resize(threadsNumber);
    for (int i = 0; i < threadsNumber; i++) {
        threadFiles[i].second = 0;
    }
    for (int j = 0; j < (int)allFiles.size(); j++) {
        sumSize += allFiles[j]->size();
        int minInd = 0;
        qint64 minSize = threadFiles[0].second;
        for (int i = 1; i < threadsNumber; i++) {
            if (threadFiles[i].second < minSize) {
                minInd = i;
                minSize = threadFiles[i].second;
            }
        }
        threadFiles[minInd].first.push_back(allFiles[j]);
        threadFiles[minInd].second += allFiles[j]->size();
    }
    for (int i = 0; i < threadsNumber; i++) {
        std::reverse(threadFiles[i].first.begin(),threadFiles[i].first.end());
    }

    shift = 0;
    for (int i = 22; i < 64; i++) {
        if ((sumSize>>i) > 0) {
            shift++;
        } else {
            break;
        }
    }
    checkedFilesSize = 0;
}

void MainWindow::search() {

    ui->statusBar->showMessage("Идет поиск.");

    QProgressDialog LoadingWindow;
    if (sumSize > (1 << 19)) {
        LoadingWindow.setLabelText("Идет поиск одинаковых файлов.\nПожалуйста, подождите...");
        LoadingWindow.setMinimumSize(300, 150);
        LoadingWindow.setMaximumSize(400, 250);

        connect(&LoadingWindow, SIGNAL(canceled()), &watcher, SLOT(cancel()));
        connect(&LoadingWindow, SIGNAL(canceled()), this, SLOT(setFlag()));
        connect(&watcher, SIGNAL(finished()), &LoadingWindow, SLOT(reset()));
        connect(&watcher, SIGNAL(progressValueChanged(int)), &LoadingWindow, SLOT(setValue(int)));
    }
    watcher.setFuture(QtConcurrent::map(threadFiles,&MainWindow::findEqual));

    if (sumSize > (1 << 19)) {
        LoadingWindow.setRange(0, sumSize>>shift);
        LoadingWindow.exec();
    }

    watcher.waitForFinished();
    LoadingWindow.hide();
    if (watcher.isCanceled()) {
        filesByHash.clear();
        threadFiles.clear();
        allFiles.clear();
        checkedFilesSize = 0;
        sumSize = 0;
        QMessageBox::warning(this, "ОТМЕНА", "Поиск отменен.");
        ui->statusBar->showMessage("Поиск отменен.");
    } else if (filesByHash.size() == 0) {
        filesByHash.clear();
        threadFiles.clear();
        allFiles.clear();
        checkedFilesSize = 0;
        sumSize = 0;
        QMessageBox::warning(this, "Удача!", "Нет одинаковых файлов");
        ui->statusBar->showMessage("Нет одинаковых файлов");
    } else {
        for (auto& cur : filesByHash) {
            if (cur.second.size() > 1) {
                finalWindow.files.push_back(cur.second);
            }
        }
        if (finalWindow.files.size() == 0) {
            filesByHash.clear();
            threadFiles.clear();
            allFiles.clear();
            checkedFilesSize = 0;
            sumSize = 0;
            QMessageBox::warning(this, "Удача!", "Нет одинаковых файлов");
            ui->statusBar->showMessage("Нет одинаковых файлов");
        } else {
            hide();
            finalWindow.buttons.clear();
            finalWindow.globalDir = ui->lineEdit_Directory->text();
            emit this->openFinalWindow();
            QMessageBox::about(this, "OK", "Поиск завершен.");
            ui->statusBar->showMessage("Поиск завершен.");
        }
    }
    threadsWasCancelled = false;
    ui->checkBox_Threads->setEnabled(true);
    ui->pushButton_Browse->setEnabled(true);
    ui->pushButton_Start->setEnabled(true);
    ui->lineEdit_Directory->setEnabled(true);
}

void MainWindow::closeFinalWindow() {
    filesByHash.clear();
    threadFiles.clear();
    allFiles.clear();
    checkedFilesSize = 0;
    sumSize = 0;
    ui->lineEdit_Directory->setText("");
    ui->statusBar->showMessage("");
    show();
}
