#ifndef FILESDIALOG_H
#define FILESDIALOG_H

#include <QDialog>
#include <QFile>
#include <vector>
#include <QString>
#include <QColor>
#include <QBrush>
#include <QMessageBox>

namespace Ui {
class FilesDialog;
}

class FilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilesDialog(QWidget *parent = 0);
    ~FilesDialog();

    QString globalDir;
    static std::vector <QFile*> eqFiles;
    static std::vector <QString> delFiles;

    void openWindow();

private slots:
    void on_pushButton_deleteFiles_clicked();

private:
    Ui::FilesDialog *ui;
};

#endif // FILESDIALOG_H
