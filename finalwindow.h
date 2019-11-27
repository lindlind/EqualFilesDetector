#ifndef FINALWINDOW_H
#define FINALWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <vector>
#include <QFile>
#include <QString>
#include "filesdialog.h"

namespace Ui {
class FinalWindow;
}

class ButtonWithId : public QPushButton {
    Q_OBJECT
public:

    int id;
    ButtonWithId( const QString& text, int id, QWidget* parent = 0 );

};

class FinalWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FinalWindow(QWidget *parent = 0);
    ~FinalWindow();

    QString globalDir;
    static std::vector <std::vector <QFile*> > files;
    static std::vector <ButtonWithId*> buttons;

public slots:
    void showWindow();

    void buildWindow();

    void buttonClicked();

private slots:
    void on_pushButton_2_clicked();

signals:
    void closeWindow();

private:
    Ui::FinalWindow *ui;
};

#endif // FINALWINDOW_H
