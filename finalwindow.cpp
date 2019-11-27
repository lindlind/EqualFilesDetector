#include "finalwindow.h"
#include "ui_finalwindow.h"
#include <QWidget>

FinalWindow::FinalWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FinalWindow)
{
    ui->setupUi(this);
}

FinalWindow::~FinalWindow()
{
    delete ui;
}

ButtonWithId::ButtonWithId( const QString& text, int num, QWidget* parent ) : QPushButton( text, parent ) {
    id = num;
}

std::vector <std::vector <QFile*> > FinalWindow::files;
std::vector <ButtonWithId*> FinalWindow::buttons;

void FinalWindow::buildWindow() {
    int i = 0;
    for (std::vector <QFile*>& equalFiles : files) {

        qint64 sz = equalFiles[0]->size();
        QString b = "б";

        if (sz > (1 << 11)) {
            sz = sz >> 10;
            b = "Кб";
            if (sz > (1 << 11)) {
                sz = sz >> 10;
                b = "Мб";
                if (sz > (1 << 11)) {
                    sz = sz >> 10;
                    b = "Гб";
                }
            }
        }

        QString bText = ("Файлов: " + QString::number(equalFiles.size()) + "\nРазмер файла: " + QString::number(sz) + b);

        //bText = QString::number(i + 1) + "] " + bText;

        buttons.push_back(new ButtonWithId(bText, i));
        buttons[i]->setMinimumSize(150, 55);
        buttons[i]->setMaximumSize(250, 55);
        ui->gridLayout->addWidget(buttons[i], i/4, i%4);
        connect(buttons[i], SIGNAL(clicked()), this, SLOT(buttonClicked()));
        i++;
        if (i == 5000 && files.size() > 7000) {
            break;
        }
    }
}

void FinalWindow::showWindow() {

    std::sort(files.begin(), files.end(), [] (const std::vector <QFile*> &a, const std::vector <QFile*> &b) { return a[0]->size() < b[0]->size(); });

    if (files.size() > 7000) {
        ui->statusbar->showMessage("Показано 5000 групп из " + QString::number(files.size()) + ".");
    } else {
        ui->statusbar->showMessage("Найдено " + QString::number(files.size()) + " групп.");
    }
    buildWindow();
    show();
}

void FinalWindow::buttonClicked() {
    ButtonWithId *button = (ButtonWithId*)sender();

    FilesDialog dialog;
    dialog.setModal(true);
    sort(files[button->id].begin(), files[button->id].end(), [] (QFile* const &a, QFile* const &b) {return a->fileName() < b->fileName(); });
    dialog.eqFiles = files[button->id];
    dialog.delFiles.clear();
    dialog.globalDir = this->globalDir;
    dialog.openWindow();
    dialog.exec();
    sort(dialog.delFiles.begin(), dialog.delFiles.end());
    std::vector <QFile*> buf;
    int i = 0, j = 0;
    for (; i < files[button->id].size(); i++) {
        if (j < dialog.delFiles.size() && files[button->id][i]->fileName() == dialog.delFiles[j]) {
            j++;
            continue;
        }
        buf.push_back(files[button->id][i]);
    }
    swap(buf, files[button->id]);
    button->setText("Файлов: " + QString::number(files[button->id].size()) + "\n" + button->text().split("\n")[1]);
    if (files[button->id].size() == 0) {
        button->setEnabled(false);
    }
}

void FinalWindow::on_pushButton_2_clicked()
{
    hide();
    for (int i = 0; i < buttons.size(); i++) {
        QWidget* widget = ui->gridLayout->itemAtPosition(i/4, i%4)->widget();
        if (widget) {
            ui->gridLayout->removeWidget(widget);
            delete widget;
        }
    }
    files.clear();
    buttons.clear();
    emit this->closeWindow();
}
