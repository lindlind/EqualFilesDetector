#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QProgressDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QString>
#include <vector>
#include <QThread>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QCryptographicHash>
#include "finalwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static bool threadsWasCancelled;
    static int threadsNumber, shift;
    static qint64 checkedFilesSize, sumSize;
    static QMutex mutex;
    static std::vector <QFile*> allFiles;
    static QFutureWatcher<void> watcher, preparingWatcher;
    static std::map <QByteArray, std::vector <QFile*> > filesByHash;
    static std::vector <std::pair <std::vector <QFile*>, qint64> > threadFiles;

    static void getAllFiles(QString directory);
    static void deleteUniqueFiles();
    static void preparing(QString dir);
    static void findEqual(std::pair <std::vector <QFile*>, qint64> &pairs);

    FinalWindow finalWindow;

private slots:
    void on_checkBox_Threads_clicked();

    void on_pushButton_Browse_clicked();

    void on_pushButton_Start_clicked();

    void search();

    void setFlag();

    void closeFinalWindow();

signals:
    void openFinalWindow();


private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
