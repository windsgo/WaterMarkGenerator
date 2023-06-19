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
    void on_pbLoadFile_clicked();

    void on_pbSaveFile_clicked();

    void on_pbChooseFile_clicked();

    void on_pbConfirmManualString_clicked();

    void on_pbCacheCurrentMark_clicked();

private:
    void _updateGenerateStringText();

    bool _updateWaterMarkedPicture();

    void _reset();

private:
    enum class GenerateStringType {
        ShootDateTime,
        BirthDateTime,
        ModifyDateTime,
        MaunalInput
    } current_generate_string_type_ = GenerateStringType::ModifyDateTime;

    enum class State {
        FileUnloaded,
        FileLoaded
    } current_state_;

private:
    Ui::MainWindow *ui;

    QImage current_cached_img_;
    QImage current_display_img_;

    QString loaded_filename_;

    QButtonGroup *btn_group_;

    WaterMarkGenerator mark_generator_;

    QString error_msg_;
};
#endif // MAINWINDOW_H
