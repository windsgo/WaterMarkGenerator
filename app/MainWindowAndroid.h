#ifndef MAINWINDOWANDROID_H
#define MAINWINDOWANDROID_H

#include <QMainWindow>

#ifdef USE_QEXIF_LIB
#include "qexifimageheader.h"
#include "qmetadata.h"
#else // !USE_QEXIF_LIB
#include "exiv2/exiv2.hpp"
#endif // USE_QEXIF_LIB

#include "../src/WaterMarkHolder.h"

#include <QButtonGroup>

namespace Ui {
class MainWindowAndroid;
}

class MainWindowAndroid : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindowAndroid(QWidget *parent = nullptr);
    ~MainWindowAndroid();

private:
    void _do_loadFile();
    void _do_saveFile();
    void _do_chooseFile();
    void _do_confirmEdit();

private:
    bool _updateWaterMarkedPicture();

    void _updateHolderSettings();

    void _reset();

private:

    enum class State {
        FileUnloaded,
        FileLoaded
    } current_state_;

private:
    Ui::MainWindowAndroid *ui;

    QString loaded_filename_;
#ifdef USE_QEXIF_LIB
    // QEXIF lib do not support cache exif message ... (disabled copy ctor)
#else // !USE_QEXIF_LIB
    Exiv2::ExifData loaded_img_exif_;
#endif // USE_QEXIF_LIB

    wmg::WaterMarkHolder::ptr wm_holder_ {nullptr};

    wmg::WaterMarkDesc using_desc_;

    QString error_msg_;

    bool allow_autoupdate_ {true};

    QButtonGroup* btn_group_;
};

#endif // MAINWINDOWANDROID_H
