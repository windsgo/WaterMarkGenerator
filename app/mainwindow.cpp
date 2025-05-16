#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QImage>
#include <QPixmap>
#include <QDebug>

#include <QFileDialog>
#include <QMessageBox>

#include "../src/Utils.h"

#include "version.h"

#define RETURN_ERROR(err_msg) { error_msg_ = (err_msg); return false; }

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString version_str = QString("V%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH);
    // 或者 QString(APP_VERSION_STRING)
    ui->lbVersion->setText(version_str);

    this->wm_holder_ = std::make_shared<wmg::WaterMarkHolder>();

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
        _do_modify_current_row_str();
        _updateWaterMarkedPicture();
    });

    connect(ui->pbGetBirthTime, &QPushButton::clicked, this, [&]() {
        auto str_opt = wmg::Utils::getBirthTimeStringOfFile(loaded_filename_);

        if (!str_opt) {
            QMessageBox::warning(this, tr("获取创建时间失败"), tr("获取创建时间失败, 照片可能没有记录此信息"));
            return;
        }

        ui->leString->setText(*str_opt);
        _do_modify_current_row_str();
        _updateWaterMarkedPicture();
    });

    connect(ui->pbGetModifyTime, &QPushButton::clicked, this, [&]() {
        auto str_opt = wmg::Utils::getLastModifyTimeStringOfFile(loaded_filename_);

        if (!str_opt) {
            QMessageBox::warning(this, tr("获取修改时间失败"), tr("获取修改时间失败, 照片可能没有记录此信息"));
            return;
        }

        ui->leString->setText(*str_opt);
        _do_modify_current_row_str();
        _updateWaterMarkedPicture();
    });

    connect(ui->rbDigitalFont, &QRadioButton::clicked, this, [&](){
        ui->cbDigitalFonts->setEnabled(true);
        ui->fcbSystemFonts->setEnabled(false);

        _do_modify_current_row_font();
        _updateWaterMarkedPicture();
    });
    connect(ui->rbSystemFont, &QRadioButton::clicked, this, [&](){
        ui->cbDigitalFonts->setEnabled(false);
        ui->fcbSystemFonts->setEnabled(true);

        _do_modify_current_row_font();
        _updateWaterMarkedPicture();
    });

    connect(ui->cbDigitalFonts, &QComboBox::currentTextChanged, this, [&](){
        _do_modify_current_row_font();
        _updateWaterMarkedPicture();
    });
    connect(ui->fcbSystemFonts, &QFontComboBox::currentTextChanged, this, [&](){
        _do_modify_current_row_font();
        _updateWaterMarkedPicture();
    });

    connect(ui->dsbFontPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&](){
                if (!allow_autoupdate_) {
                    // 阻挡在点list的时候的自动刷新, 这种时候不需要刷新水印
                    return;
                }
                _do_modify_current_row_pos();
                _updateWaterMarkedPicture();
            });
    connect(ui->dsbBottomPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&](){
                if (!allow_autoupdate_) {
                    // 阻挡在点list的时候的自动刷新, 这种时候不需要刷新水印
                    return;
                }
                _do_modify_current_row_pos();
                _updateWaterMarkedPicture();
            });
    connect(ui->dsbRightPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [&](){
                if (!allow_autoupdate_) {
                    // 阻挡在点list的时候的自动刷新, 这种时候不需要刷新水印
                    return;
                }
                _do_modify_current_row_pos();
                _updateWaterMarkedPicture();
            });

    connect(ui->pbLoadFile, &QPushButton::clicked, this, &MainWindow::_do_loadFile);
    connect(ui->pbSaveFile, &QPushButton::clicked, this, &MainWindow::_do_saveFile);
    connect(ui->pbChooseFile, &QPushButton::clicked, this, &MainWindow::_do_chooseFile);
    connect(ui->pbConfirmEdit, &QPushButton::clicked, this, &MainWindow::_do_confirmEdit);

    // init list
    ui->listWidgetWM->clear();
    ui->listWidgetWM->setSelectionMode(QListWidget::SelectionMode::SingleSelection);
    ui->listWidgetWM->setSelectionBehavior(QListWidget::SelectionBehavior::SelectRows);

    connect(ui->pbAddNewWM, &QPushButton::clicked, this, [this]() {
        qDebug() << "pbAddNewWM";
        _do_add_one_wm();
    });

    connect(ui->pbDeleteSelectedWM, &QPushButton::clicked, this, [this]() {
        qDebug() << "ui->pbDeleteSelectedWM";
        Q_ASSERT(ui->listWidgetWM->count() != wm_holder_->getDesciptionList().size());
        if (ui->listWidgetWM->count() != wm_holder_->getDesciptionList().size()) {
            qCritical() << "ui->listWidgetWM->count() != wm_holder_->getDesciptionList().size()";
        }

        if (ui->listWidgetWM->count() == 0) {
            return;
        }

        auto current_row = ui->listWidgetWM->currentRow();

        if (current_row < 0) {
            return;
        }

        wm_holder_->getDesciptionList().removeAt(current_row);
        _updateListWidgetAccordingToWMHolder();

        if (ui->listWidgetWM->count() > 0) {
            int should_select_row = current_row;
            if (should_select_row < 0) {
                should_select_row = 0;
            } else if (should_select_row > ui->listWidgetWM->count() - 1) {
                should_select_row = ui->listWidgetWM->count() - 1;
            }

            ui->listWidgetWM->setCurrentRow(should_select_row);
        }
        _do_update_info_according_to_current_select_row();

        _updateWaterMarkedPicture();
    });

    connect(ui->listWidgetWM, &QListWidget::itemClicked, this, [this]() {
        _do_update_info_according_to_current_select_row();
        _updateWaterMarkedPicture(); // redraw the border
    });

    _reset();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::_updateWaterMarkedPicture()
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

    QSet<int> current_index_set;
    current_index_set.insert( ui->listWidgetWM->currentRow() );
    auto wm_image = wm_holder_->generateWaterMarkedImage(current_index_set);

    if (wm_image.isNull()) {
        RETURN_ERROR(tr("生成水印失败, 图片文件错误或绘图错误"));
    }

    ui->lbPicture->setPixmap(QPixmap::fromImage(wm_image).scaled(ui->lbPicture->size(),
                                                                 Qt::KeepAspectRatio,
                                                                 Qt::SmoothTransformation));

    qDebug() << "_updateWaterMarkedPicture exit";
    return true;
}

void MainWindow::_reset()
{
    qDebug() << "_reset";
    current_state_ = State::FileUnloaded;

    ui->leString->clear();

    wm_holder_->reset();
    _updateListWidgetAccordingToWMHolder();

    loaded_filename_.clear();
#ifndef USE_QEXIF_LIB
    loaded_img_exif_.clear();
#endif // !USE_QEXIF_LIB

    ui->lbPicture->clear();
    ui->lbPicture->setText(tr("请载入图片"));

    // dpi and quality default
    ui->sb_export_dpi->setValue(300);
    ui->sb_export_jpg_quality->setValue(100);
}

void MainWindow::_updateListWidgetAccordingToWMHolder()
{
    qDebug() << "_updateListWidgetAccordingToWMHolder";
    int old_row = ui->listWidgetWM->currentRow();

    ui->listWidgetWM->clear();
    const auto& desc_list = wm_holder_->getDesciptionList();

    for (int i = 0; i < desc_list.size(); ++i) {
        const auto& desc = desc_list.at(i);
        ui->listWidgetWM->addItem(QString("%0 - [%1]").arg(i).arg(desc.str));
    }

    ui->listWidgetWM->setCurrentRow(old_row);
    qDebug() << "_updateListWidgetAccordingToWMHolder exit";
}

// template <typename TAG_T>
// static void func_taglist(QExifImageHeader * eh, const QList<TAG_T>& tags) {
//     for (int i = 0; i < tags.size(); ++i) {
//         auto tag = tags[i];
//         auto v = eh->value(tag);
//         qDebug() << "**" << i << "** tag:" << tag << ", type:" << v.type()
//                  << ", str:" << v.toString()
//                  << ", rational:" << v.toRational().first << v.toRational().second
//                  << ", ushort" << v.toShort();
//     }
// };

void MainWindow::_do_loadFile()
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

    wm_holder_->setImage(image);
    wm_holder_->getDesciptionList().clear();
    _updateListWidgetAccordingToWMHolder();

    current_state_ = State::FileLoaded;
    loaded_filename_ = filename;

#ifndef USE_QEXIF_LIB
    auto exif_opt = wmg::Utils::getExifDataOfPicture(filename);
    if (exif_opt) {
        loaded_img_exif_ = std::move(*exif_opt);
    } else {
        loaded_img_exif_.clear();
    }
#endif // !USE_QEXIF_LIB

    // 增加一个水印
    ui->pbGetExifTime->click();
    _do_add_one_wm();

    int dpi = 123;
#ifdef USE_QEXIF_LIB
    if (auto dpi_rational_opt = wmg::Utils::getExifDPIResolutionOfPicture(filename)) {
        dpi = static_cast<double>(dpi_rational_opt->first) / dpi_rational_opt->second;
    }
#else // !USE_QEXIF_LIB
    if (loaded_img_exif_.findKey(Exiv2::ExifKey{WMG_EXIFDATA_KEYSTR_EXIF_XRESOLUTION}) != loaded_img_exif_.end()) {
        auto rational = loaded_img_exif_[WMG_EXIFDATA_KEYSTR_EXIF_XRESOLUTION].value().toRational();
        dpi = static_cast<double>(rational.first) / rational.second;
    }
#endif // USE_QEXIF_LIB
    else {
        // use qimage things
        int dpm = image.dotsPerMeterX();
        dpi = dpm * 0.0254;
    }
    ui->sb_export_dpi->setValue(dpi);

    // auto emd = QMetaData{filename};
    // auto eh_list = emd.exifImageHeaderList();
    // if (!eh_list) {
    //     qDebug() << "!eh_list";
    // } else {
    //     qDebug() << "eh_list->size():" << eh_list->size();
    //     if (eh_list->size() > 0) {
    //         auto eh0 = eh_list->at(0);

    //         auto eh0_itags = eh0->imageTags();
    //         qDebug() << "eh0_itags.size():" << eh0_itags.size();
    //         func_taglist(eh0, eh0_itags);

    //         auto eh0_etags = eh0->extendedTags();
    //         qDebug() << "eh0_etags.size():" << eh0_etags.size();
    //         func_taglist(eh0, eh0_etags);
    //     }
    // }

    QMessageBox::information(this, tr("通知"),
                             tr("载入文件成功"));

    // 更新一次水印
    if (!_updateWaterMarkedPicture()) {
        QMessageBox::warning(this, tr("错误"),
                             error_msg_);
        return;
    }
}

#ifdef USE_QEXIF_LIB
static bool _write_dpi_and_copy_exifdata_to_file(const QString& src_filename, const QString& target_filename, int dpi)
{
    {
        QFileInfo fileinfo(src_filename);
        if (!fileinfo.exists()) {
            return false;
        }
    }

    {
        QFileInfo fileinfo(target_filename);
        if (!fileinfo.exists()) {
            return false;
        }
    }

    // read source metadata
    auto src_metadata = QMetaData(src_filename);
    auto p_src_exif_header_list = src_metadata.exifImageHeaderList();
    if (!p_src_exif_header_list || p_src_exif_header_list->empty()) {
        qWarning() << "_write_dpi_and_copy_exifdata_to_file" << "!p_src_exif_header_list || p_src_exif_header_list->empty()";
        return false;
    }

    auto p_src_exif_header_0 = p_src_exif_header_list->at(0);
    if (!p_src_exif_header_0) {
        qWarning() << "_write_dpi_and_copy_exifdata_to_file" << "!p_src_exif_header_0";
        return false;
    }

    // read target metadata
    auto tar_metadata = QMetaData(target_filename);
    auto p_tar_exif_header_list = tar_metadata.exifImageHeaderList();
    if (!p_tar_exif_header_list) {
        qWarning() << "_write_dpi_and_copy_exifdata_to_file" << "!p_tar_exif_header_list";
        return false;
    }

    if (p_tar_exif_header_list->empty()) {
        qWarning() << "_write_dpi_and_copy_exifdata_to_file" << "p_tar_exif_header_list->empty()";
        qDebug() << "insert empty exif header to target: " << target_filename;

        p_tar_exif_header_list->append(new QExifImageHeader{});
    }

    auto p_tar_exif_header_0 = p_tar_exif_header_list->at(0);
    if (!p_tar_exif_header_0) {
        qWarning() << "_write_dpi_and_copy_exifdata_to_file" << "!p_tar_exif_header_0";
        return false;
    }
    *p_tar_exif_header_0 = *p_src_exif_header_0;

    for (const auto& etag : tar_metadata.exifImageHeaderList()->at(0)->extendedTags()) {
        // p_tar_exif_header_0->setValue(itag, p_src_exif_header_0->value(itag));
        qDebug() << "etag: " << etag <<  tar_metadata.exifImageHeaderList()->at(0)->value(etag).toString();
    }

    // // change dpi
    // p_src_exif_header_0->setValue(QExifImageHeader::ImageTag::XResolution, QExifURational{dpi, 1});
    // p_src_exif_header_0->setValue(QExifImageHeader::ImageTag::YResolution, QExifURational{dpi, 1});
    // p_src_exif_header_0->setValue(QExifImageHeader::ImageTag::ResolutionUnit, uint16_t{2});
    // p_src_exif_header_0->setThumbnail(QImage{});

    return tar_metadata.saveToJpeg(target_filename);
}
#endif // USE_QEXIF_LIB

void MainWindow::_do_saveFile()
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

    QFileInfo input_fileinfo(ui->leLoadFilename->text());
    QString dir = input_fileinfo.absolutePath();
    QString filename = QFileDialog::getSaveFileName(this, tr("请选择保存图片"), dir, tr("Images (*.jpg);; All Files(*)"));
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

    if (!wm_image.save(filename, nullptr, ui->sb_export_jpg_quality->value())) {
        QMessageBox::warning(this, tr("错误"), tr("保存图片失败"));
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
#endif // USE_QEXIF_LIB

    QMessageBox::information(this, tr("通知"),
                             tr("保存文件成功: %0").arg(filename));
}

void MainWindow::_do_chooseFile()
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

void MainWindow::_do_confirmEdit()
{
    qDebug() << "_do_confirmEdit";
    _do_modify_current_row_str();
    _do_modify_current_row_pos();
    _do_modify_current_row_font();

    _updateWaterMarkedPicture();

    // if (!_updateWaterMarkedPicture()) {
    //     QMessageBox::warning(this, tr("错误"), error_msg_);
    //     return;
    // }
}

void MainWindow::_do_add_one_wm()
{
    qDebug() << "_do_add_one_wm";
    Q_ASSERT(ui->listWidgetWM->count() == wm_holder_->getDesciptionList().size());
    if (ui->listWidgetWM->count() != wm_holder_->getDesciptionList().size()) {
        qCritical() << "ui->listWidgetWM->count() != wm_holder_->getDesciptionList().size()";
    }

    auto current_row = ui->listWidgetWM->currentRow();

    wmg::WaterMarkDesc desc;
    if (current_row >= 0) {
        desc = wm_holder_->getDesciptionList().at(current_row);
        desc.from_bottom_percent += 5;
        desc.validate_self();
    }

    desc.str = ui->leString->text();

    wm_holder_->getDesciptionList().append(desc);
    _updateListWidgetAccordingToWMHolder();
    ui->listWidgetWM->setCurrentRow(ui->listWidgetWM->count() - 1);
    _do_update_info_according_to_current_select_row();

    _updateWaterMarkedPicture();
    qDebug() << "_do_add_one_wm exit";
}

void MainWindow::_do_update_info_according_to_current_select_row()
{
    qDebug() << "_do_update_info_according_to_current_select_row";
    if (ui->listWidgetWM->count() == 0) {
        return;
    }

    int row = ui->listWidgetWM->currentRow();
    if (row < 0) {
        qDebug() << "_do_update_info_according_to_current_select_row exit(row<0)";
        return;
    }
    const auto& desc = wm_holder_->getDesciptionList().at(row);

    // only update pos and text
    allow_autoupdate_ = false;
    ui->leString->setText(desc.str);

    ui->dsbBottomPercent->setValue(desc.from_bottom_percent);
    ui->dsbRightPercent->setValue(desc.from_right_percent);
    ui->dsbFontPercent->setValue(desc.font_percent); // these may trigger auto update
    allow_autoupdate_ = true;
    qDebug() << "_do_update_info_according_to_current_select_row exit";
}

void MainWindow::_do_modify_current_row_font()
{
    qDebug() << "_do_modify_current_row_font";
    int row = ui->listWidgetWM->currentRow();
    if (row < 0) {
        qDebug() << "_do_modify_current_row_font exit(row<0)";
        return;
    }
    auto& desc = wm_holder_->getDesciptionList()[row];

    /* 选择字体 */
    if (ui->rbDigitalFont->isChecked()) {
        std::optional<QFont> font_opt;
        if (ui->cbDigitalFonts->currentIndex() == 0) {
            /* traditional font */
            font_opt = wmg::Utils::getFontFromFile(":/font/MTC-7-Segment.ttf");
        } else {
            /* enhanced font */
            font_opt = wmg::Utils::getFontFromFile(":/font/LCD2U-4.ttf");
        }
        if (font_opt) {
            desc.font = *font_opt;
        }
    } else { /* ui->rbSystemFont->isChecked() */
        desc.font = ui->fcbSystemFonts->currentFont();
    }

    qDebug() << "_do_modify_current_row_font exit";
}

void MainWindow::_do_modify_current_row_str()
{
    qDebug() << "_do_modify_current_row_str";
    int row = ui->listWidgetWM->currentRow();
    if (row < 0) {
        qDebug() << "_do_modify_current_row_str exit(row<0)";
        return;
    }
    auto& desc = wm_holder_->getDesciptionList()[row];

    desc.str = ui->leString->text();

    _updateListWidgetAccordingToWMHolder(); // update str display
    qDebug() << "_do_modify_current_row_str exit";
}

void MainWindow::_do_modify_current_row_pos()
{
    qDebug() << "_do_modify_current_row_pos";
    int row = ui->listWidgetWM->currentRow();
    if (row < 0) {
        qDebug() << "_do_modify_current_row_pos exit(row<0)";
        return;
    }
    auto& desc = wm_holder_->getDesciptionList()[row];

    desc.font_percent = ui->dsbFontPercent->value();
    desc.from_bottom_percent = ui->dsbBottomPercent->value();
    desc.from_right_percent = ui->dsbRightPercent->value();

    qDebug() << "_do_modify_current_row_pos exit";
}
