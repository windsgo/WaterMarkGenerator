#include "MainWindowAndroid.h"
#include "ui_MainWindowAndroid.h"

#include <QGuiApplication>
#include <QScreen>
#include <QImage>
#include <QPixmap>
#include <QDebug>

#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

#include <QRegularExpressionValidator>
#include <QRegularExpression>

#include "../src/Utils.h"

#include "version.h"

#define RETURN_ERROR(err_msg) { error_msg_ = (err_msg); this->statusBar()->showMessage(error_msg_, 5000); return false; }

MainWindowAndroid::MainWindowAndroid(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowAndroid)
{
    ui->setupUi(this);

    QString version_str = QString("版本：%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH);
    // 或者 QString(APP_VERSION_STRING)
    ui->lbVersion->setText(version_str);

    // 创建正则表达式
    QRegularExpression regex("^[A-Fa-f0-9]{6}$");

    // 创建验证器
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regex, this);

    // 设置验证器到 QLineEdit
    ui->leCustomColor->setValidator(validator);

    this->wm_holder_ = std::make_shared<wmg::WaterMarkHolder>();

#if 0
    connect(ui->pbGetExifTime, &QPushButton::clicked, this, [&]() {
#ifdef USE_QEXIF_LIB
        auto str_opt = wmg::Utils::getShootDateTimeStringOfPicture(loaded_filename_);
#else // !USE_QEXIF_LIB
        auto str_opt = wmg::Utils::getShootDateTimeStringFromExifData(loaded_img_exif_);
#endif // USE_QEXIF_LIB

        if (!str_opt) {
            QMessageBox::warning(this, tr("获取拍摄时间失败"), tr("获取拍摄时间失败, 照片可能没有记录此信息"));
            return;
        }

        ui->leString->setText(*str_opt);
        _updateWaterMarkedPicture();
    });

    connect(ui->pbGetBirthTime, &QPushButton::clicked, this, [&]() {
        auto str_opt = wmg::Utils::getBirthTimeStringOfFile(loaded_filename_);

        if (!str_opt) {
            QMessageBox::warning(this, tr("获取创建时间失败"), tr("获取创建时间失败, 照片可能没有记录此信息"));
            return;
        }

        ui->leString->setText(*str_opt);
        _updateWaterMarkedPicture();
    });
#endif // 安卓中这些好像都没用, 让用户手搓吧。。。

    connect(ui->pbGetModifyTime, &QPushButton::clicked, this, [&]() {
        auto str_opt = wmg::Utils::getLastModifyTimeStringOfFile(loaded_filename_);

        if (!str_opt) {
            QMessageBox::warning(this, tr("获取修改时间失败"), tr("获取修改时间失败, 照片可能没有记录此信息"));
            return;
        }

        ui->leString->setText(*str_opt);
        _updateWaterMarkedPicture();
    });

    connect(ui->pbSaveFile, &QPushButton::clicked, this, [this](){
        _do_saveFile();
    });

    connect(ui->pbLoadFile, &QPushButton::clicked, this, [this](){
        _do_loadFile();
    });

    connect(ui->pbChooseFile, &QPushButton::clicked, this, [this](){
        _do_chooseFile();
    });

    connect(ui->pbConfirmEdit, &QPushButton::clicked, this, [this](){
        _do_confirmEdit();
    });

    btn_group_ = new QButtonGroup(this);
    btn_group_->addButton(ui->rbOrange, 0);
    btn_group_->addButton(ui->rbYellow, 1);
    btn_group_->addButton(ui->rbCustom, 2);
    btn_group_->setExclusive(true);
    ui->rbOrange->setChecked(true);

    connect(btn_group_, &QButtonGroup::idToggled, this, [this](int id, bool checked) {
        if (!checked) {
            return;
        }

        qDebug() << "clicked" << id;
        _updateWaterMarkedPicture();
    });

    connect(ui->leCustomColor, &QLineEdit::editingFinished, this, [this]() {
        _updateWaterMarkedPicture();
    });

    connect(ui->pbAdjustBigger, &QPushButton::clicked, this, [this](){
        using_desc_.font_percent += 0.5;
        using_desc_.validate_self();

        _updateWaterMarkedPicture();
        QCoreApplication::processEvents();
        this->repaint();
    });

    connect(ui->pbAdjustSmaller, &QPushButton::clicked, this, [this](){
        using_desc_.font_percent -= 0.5;
        using_desc_.validate_self();

        _updateWaterMarkedPicture();
        this->repaint();
        QCoreApplication::processEvents();
    });

    connect(ui->pbAdjustUp, &QPushButton::clicked, this, [this](){
        using_desc_.from_bottom_percent += 1.0;
        using_desc_.validate_self();

        _updateWaterMarkedPicture();
        this->repaint();
        QCoreApplication::processEvents();
    });

    connect(ui->pbAdjustDown, &QPushButton::clicked, this, [this](){
        using_desc_.from_bottom_percent -= 1.0;
        using_desc_.validate_self();

        _updateWaterMarkedPicture();
        this->repaint();
        QCoreApplication::processEvents();
    });

    connect(ui->pbAdjustLeft, &QPushButton::clicked, this, [this](){
        using_desc_.from_right_percent += 1.0;
        using_desc_.validate_self();

        _updateWaterMarkedPicture();
        this->repaint();
        QCoreApplication::processEvents();
    });

    connect(ui->pbAdjustRight, &QPushButton::clicked, this, [this](){
        using_desc_.from_right_percent -= 1.0;
        using_desc_.validate_self();

        _updateWaterMarkedPicture();
        this->repaint();
        QCoreApplication::processEvents();
    });

    _reset();
}

MainWindowAndroid::~MainWindowAndroid()
{
    delete ui;
}

void MainWindowAndroid::_do_loadFile()
{
    qDebug() << "_do_loadFile";
    QString filename = ui->leLoadFilename->text();

    // 检查文件名合法性
    QFileInfo fileinfo(filename);
    if (filename.isEmpty() || filename.isNull() || !fileinfo.exists()) {
        QMessageBox::warning(this, tr("错误"),
                             tr("文件名错误或不存在"));
        return;
    }

    QImage image{filename};
    qDebug() << image.format();
    if( image.isNull() ) {
        QMessageBox::warning(this, tr("错误"),
                             tr("载入文件错误"));
        _reset();
        return;
    }

    // 设置状态
    current_state_ = State::FileLoaded;
    loaded_filename_ = filename;

    // 获取一次字符串
    // ui->pbGetExifTime->click();
    ui->pbGetModifyTime->click();

    // 设置一次Holder图片
    wm_holder_->setImage(image);

    // 设置一次Holder参数
    _updateHolderSettings();

#ifndef USE_QEXIF_LIB
#ifndef BUILD_FOR_ANDROID
    auto exif_opt = wmg::Utils::getExifDataOfPicture(filename);
    if (exif_opt) {
        loaded_img_exif_ = std::move(*exif_opt);
    } else {
        loaded_img_exif_.clear();
    }
#endif // BUILD_FOR_ANDROID
#endif // !USE_QEXIF_LIB

    int dpi = 123;
#ifdef USE_QEXIF_LIB
    if (auto dpi_rational_opt = wmg::Utils::getExifDPIResolutionOfPicture(filename)) {
        dpi = static_cast<double>(dpi_rational_opt->first) / dpi_rational_opt->second;
    } else
#else // !USE_QEXIF_LIB
#ifndef BUILD_FOR_ANDROID
    if (loaded_img_exif_.findKey(Exiv2::ExifKey{WMG_EXIFDATA_KEYSTR_EXIF_XRESOLUTION}) != loaded_img_exif_.end()) {
        auto rational = loaded_img_exif_[WMG_EXIFDATA_KEYSTR_EXIF_XRESOLUTION].value().toRational();
        dpi = static_cast<double>(rational.first) / rational.second;
    } else
#endif // BUILD_FOR_ANDROID
#endif // USE_QEXIF_LIB
    {
        // use qimage things
        int dpm = image.dotsPerMeterX();
        dpi = dpm * 0.0254;
    }
    ui->sb_export_dpi->setValue(dpi);

    QMessageBox::information(this, tr("通知"),
                             tr("载入文件成功"));

    // 更新一次水印
    if (!_updateWaterMarkedPicture()) {
        QMessageBox::warning(this, tr("错误"),
                             error_msg_);
        return;
    }
}

// const QString s_tar_dir = "/storage/emulated/0/DCIM/Camera/";
static const QString s_tar_dir_base = "/storage/emulated/0";
static const QString s_tar_dir_root = "/Pictures/WeiXin";
static const QString s_tar_dir = s_tar_dir_base + s_tar_dir_root;

static QString _get_save_filename(const QString &ori_filename)
{
    // QString save_filename;
    //#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64) || defined(Q_OS_LINUX)
    //    save_filename = ori_filename + "_watermark_";
    //#elif defined(Q_OS_ANDROID)
    //#else
    //    save_filename = ori_filename + "_watermark_";
    //#endif

    // Create dir if not exists
    QDir dir(s_tar_dir);
    if (!dir.exists()) {
        qWarning() << "dir not exists, create one." << s_tar_dir;

        bool ret = dir.mkpath(s_tar_dir);
        if (!ret) {
            qWarning() << "mkpath failed";
        } else {
            qDebug() << "mkpath success";
        }
    }

    // QString unique_str = QString::number(QDateTime::currentMSecsSinceEpoch());
    // QString save_filename = s_tar_dir + "/watermark_" + unique_str + ".jpg";

    QString save_filename = s_tar_dir + "/IMG_"
                            + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_hhmmss")) + ".jpg";

    // save_filename = "/storage/emulated/0/DCIM/Camera/watermark_";
    // save_filename = "/storage/emulated/0/Download/watermark_";

    // save_filename = save_filename + unique_str + ".jpg";

    return save_filename;
}

void MainWindowAndroid::_do_saveFile()
{
    qDebug() << "_do_saveFile";
    if (current_state_ == State::FileUnloaded) {
        QMessageBox::warning(this, tr("错误"), tr("保存文件失败, 未载入文件"));
        return;
    }

    // get no border water-marked pic
    QSet<int> none_set;
    auto wm_image = wm_holder_->generateWaterMarkedImage(none_set);

    if (wm_image.isNull()) {
        QMessageBox::warning(this, tr("错误"), tr("保存文件失败, 水印生成失败"));
        return;
    }

    // QFileInfo input_fileinfo(loaded_filename_);
    // QString dir = input_fileinfo.absolutePath();
    // QString filename = QFileDialog::getSaveFileName(this, tr("请选择保存图片"), dir, tr("Images (*.jpg);; All Files(*)"));
    // 安卓中, 直接保存在原地
    // auto file_dir = input_fileinfo.absoluteDir().path(); // 路径
    // auto input_filename_no_suffix = input_fileinfo.completeBaseName(); // 无后缀名
    // QString unique_str = QString::number( QDateTime::currentMSecsSinceEpoch() );
    // QString filename = file_dir + QDir::separator() + input_filename_no_suffix + "_" + unique_str + ".jpg";
    QString filename = _get_save_filename(loaded_filename_);
    if (filename.isEmpty() || filename.isNull())
    {
        return;
    }

    QFileInfo fileinfo(filename);
    if (fileinfo.suffix() != "jpg") {
        filename.append(".jpg");
    }

    int dpi = ui->sb_export_dpi->value();
    int dpm = dpi / 0.0254;
    wm_image.setDotsPerMeterX(dpm);
    wm_image.setDotsPerMeterY(dpm);

    if (!wm_image.save(filename, "JPG", ui->sb_export_jpg_quality->value())) {
        QMessageBox::warning(this, tr("错误"), tr("保存图片失败: %0").arg(filename));
        return;
    }

// write dpi
#ifdef USE_QEXIF_LIB
    // cannot support

// bool ret = _write_dpi_and_copy_exifdata_to_file(loaded_filename_, filename, dpi);
// if (!ret) {
//     QMessageBox::warning(this, tr("错误"), tr("保存文件时, 写入DPI失败"));
// }
#else // !USE_QEXIF_LIB

#ifndef BUILD_FOR_ANDROID
    auto temp_exif = loaded_img_exif_;
    qDebug() << "dpi:" << dpi;
    temp_exif[WMG_EXIFDATA_KEYSTR_EXIF_XRESOLUTION] = Exiv2::Rational(dpi, 1);
    temp_exif[WMG_EXIFDATA_KEYSTR_EXIF_YRESOLUTION] = Exiv2::Rational(dpi, 1);
    temp_exif[WMG_EXIFDATA_KEYSTR_EXIF_RESOLUTIONUNIT] = Exiv2::UShortValue(2);

    temp_exif[WMG_EXIFDATA_KEYSTR_THUMBNAIL_XRESOLUTION] = Exiv2::Rational(dpi, 1);
    temp_exif[WMG_EXIFDATA_KEYSTR_THUMBNAIL_YRESOLUTION] = Exiv2::Rational(dpi, 1);
    temp_exif[WMG_EXIFDATA_KEYSTR_THUMBNAIL_RESOLUTIONUNIT] = Exiv2::UShortValue(2);

    // if (temp_exif.findKey(Exiv2::ExifKey{WMG_EXIFDATA_KEYSTR_EXIF_RESOLUTIONUNIT}) == temp_exif.end()) {
    //     temp_exif[WMG_EXIFDATA_KEYSTR_EXIF_RESOLUTIONUNIT] = Exiv2::UShortValue(2);
    //     temp_exif[WMG_EXIFDATA_KEYSTR_THUMBNAIL_RESOLUTIONUNIT] = Exiv2::UShortValue(2);
    // }

    try {
        auto image = Exiv2::ImageFactory::open(filename.toLocal8Bit().toStdString());

        image->setExifData(temp_exif);
        image->writeMetadata();

    }  catch (const Exiv2::Error& e) {
        qDebug() << " Exiv2::Error: " << e.what();
        QMessageBox::warning(this, tr("错误"), tr("保存文件时, 写入DPI失败: %0").arg(e.what()));
    }
#endif

#endif // USE_QEXIF_LIB

    QMessageBox::information(this, tr("通知"),
                             tr("保存文件成功: %0\n 请前往文件管理目录%1查看!!").arg(filename).arg(s_tar_dir_root));
    this->statusBar()->showMessage(tr("保存成功:%0").arg(filename), 5000);
}

void MainWindowAndroid::_do_chooseFile()
{
    qDebug() << "_do_chooseFile";
    static QString last_dir;
    QString filename = QFileDialog::getOpenFileName(this, tr("请选择图片"), last_dir, tr("Images (*.png *.jpg);; All Files(*)"));
    if (filename.isEmpty() || filename.isNull())
    {
        return;
    }

    QFileInfo fileinfo(filename);
    last_dir = fileinfo.absolutePath();

    ui->leLoadFilename->setText(filename);

    _do_loadFile();
}

void MainWindowAndroid::_do_confirmEdit()
{
    qDebug() << "_do_confirmEdit";
    _updateWaterMarkedPicture();
}

bool MainWindowAndroid::_updateWaterMarkedPicture()
{
    qDebug() << "_updateWaterMarkedPicture";
    switch (current_state_) {
    case State::FileUnloaded:
        RETURN_ERROR(tr("生成水印失败, 未载入文件"));
    case State::FileLoaded:
        break; // continue steps outside switch
    default:
        RETURN_ERROR(tr("未知状态错误, 状态%0").arg(static_cast<int>(current_state_)));
    }

    if (wm_holder_->isNull()) {
        RETURN_ERROR(tr("生成水印失败, 图片文件读取错误"));
    }

    _updateHolderSettings();
    QSet<int> current_index_set; // dummy empty set
    auto wm_image = wm_holder_->generateWaterMarkedImage(current_index_set);

    if (wm_image.isNull()) {
        RETURN_ERROR(tr("生成水印失败, 图片文件错误或绘图错误"));
    }

    qDebug() << "before" << ui->lbPicture->size();
    QPixmap pixmap = QPixmap::fromImage(wm_image).scaled(ui->lbPicture->size(),
                                                                  Qt::KeepAspectRatio,
                                                                  Qt::SmoothTransformation);

    ui->lbPicture->setPixmap(pixmap);

    ui->lbPicture->setAlignment (Qt::AlignCenter);

    this->update();
    this->repaint();

    return true;
}

void MainWindowAndroid::_updateHolderSettings()
{
    // 获取字符串, 并把所有设置都设置到 wm_holder 中去 (完全更新一次设定)
    using_desc_.str = ui->leString->text();

    if (btn_group_->checkedId() == 0) {
        // orange
        using_desc_.color = QColor {255, 149, 21}; // "#FF9515"
    } else if (btn_group_->checkedId() == 1) {
        // yellow
        using_desc_.color = QColor {255, 255, 0}; // "#FFFF00"
    } else {
        // Custom
        QColor custom_color = QColor {"#" + QString{ui->leCustomColor->text()}};
        if (!custom_color.isValid()) {
            custom_color = QColor {255, 149, 21}; // "#FF9515"
            this->statusBar()->showMessage("自定义颜色格式错误, 格式为FFEE00这种", 5000);
        }

        using_desc_.color = custom_color;
    }
    wm_holder_->getDesciptionList().clear();
    wm_holder_->getDesciptionList().push_back(using_desc_);
}

void MainWindowAndroid::_reset()
{
    qDebug() << "_reset";
    current_state_ = State::FileUnloaded;

    ui->leString->clear();

    wm_holder_->reset();
    // _updateListWidgetAccordingToWMHolder();

    loaded_filename_.clear();
    loaded_img_exif_.clear();

    ui->lbPicture->clear();
    ui->lbPicture->setText(tr("请载入图片"));

    // dpi and quality default
    ui->sb_export_dpi->setValue(300);
    ui->sb_export_jpg_quality->setValue(100);
}
