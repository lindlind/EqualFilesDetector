#include "filesdialog.h"
#include "ui_filesdialog.h"

std::vector <QFile*> FilesDialog::eqFiles;
std::vector <QString> FilesDialog::delFiles;

FilesDialog::FilesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilesDialog)
{
    ui->setupUi(this);
}

FilesDialog::~FilesDialog()
{
    delete ui;
}

void FilesDialog::openWindow() {
    ui->listWidget_equalFiles->setSelectionMode(QAbstractItemView::MultiSelection);
    for (int i = 0; i < eqFiles.size(); i++) {
        QString fileName = eqFiles[i]->fileName();
        fileName = fileName.mid(globalDir.size() + 1, fileName.size() - globalDir.size() - 1);
        ui->listWidget_equalFiles->addItem(fileName);
    }
}

void FilesDialog::on_pushButton_deleteFiles_clicked()
{
    int ok = 0, no = 0;
    for (auto el : ui->listWidget_equalFiles->selectedItems()) {
        QFile f(globalDir + "/" + el->text());
        if (f.remove()) {
            ok++;
            delFiles.push_back(f.fileName());
            delete ui->listWidget_equalFiles->takeItem(ui->listWidget_equalFiles->row(el));
        } else {
            no++;
            el->setBackground(QBrush(QColor(170,170,170)));
        }
    }
    QMessageBox::about(this, "Удаление завершено", "Удалено файлов: " + QString::number(ok) + "\nНе удалось удалить: " + QString::number(no));
}
