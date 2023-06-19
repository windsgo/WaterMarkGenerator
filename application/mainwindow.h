#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>

#include "WaterMarkGenerator/WaterMarkGenerator.h"
#include "WaterMarkGenerator/PictureDateExtracter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pbConfirmSetting_clicked();

    void on_pbLoadFile_clicked();

    void on_pbSaveFile_clicked();

    void on_pbChooseFile_clicked();

    void on_checkBox_clicked(bool checked);

private:
    Ui::MainWindow *ui;

    QButtonGroup *btn_group_;

    WaterMarkGenerator wmg_;
};
#endif // MAINWINDOW_H
