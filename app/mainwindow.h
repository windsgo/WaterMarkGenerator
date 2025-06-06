#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>

#include "../src/WaterMarkHolder.h"

#ifdef USE_QEXIF_LIB
#include "qexifimageheader.h"
#include "qmetadata.h"
#else // !USE_QEXIF_LIB
#include "exiv2/exiv2.hpp"
#endif // USE_QEXIF_LIB

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void _do_loadFile();
    void _do_saveFile();
    void _do_chooseFile();
    void _do_confirmEdit();

    void _do_add_one_wm();

    void _do_update_info_according_to_current_select_row();

    void _do_modify_current_row_font();
    void _do_modify_current_row_str();
    void _do_modify_current_row_pos();

private:
    bool _updateWaterMarkedPicture();

    void _reset();

    void _updateListWidgetAccordingToWMHolder();

private:

    enum class State {
        FileUnloaded,
        FileLoaded
    } current_state_;

private:
    Ui::MainWindow *ui;

    QString loaded_filename_;
#ifdef USE_QEXIF_LIB
    // QEXIF lib do not support cache exif message ... (disabled copy ctor)
#else // !USE_QEXIF_LIB
    Exiv2::ExifData loaded_img_exif_;
#endif // USE_QEXIF_LIB

    wmg::WaterMarkHolder::ptr wm_holder_ {nullptr};

    QString error_msg_;

    bool allow_autoupdate_ {true};
};
#endif // MAINWINDOW_H
