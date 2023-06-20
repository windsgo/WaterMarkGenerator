#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QImage>
#include <QPixmap>
#include <QDebug>

#include <QFileDialog>
#include <QMessageBox>

#define RETURN_ERROR(err_msg) { error_msg_ = (err_msg); return false; }

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    btn_group_ = new QButtonGroup(this);
    btn_group_->addButton(ui->rbShootTime, 0);
    btn_group_->addButton(ui->rbBirthTime, 1);
    btn_group_->addButton(ui->rbModifyTime, 2);
    btn_group_->addButton(ui->rbManualInput, 3);

    connect(ui->rbShootTime, &QRadioButton::clicked, this, [&](){
        current_generate_string_type_ = GenerateStringType::ShootDateTime;
        ui->leString->setReadOnly(true);

        _updateWaterMarkedPicture();
    });

    connect(ui->rbBirthTime, &QRadioButton::clicked, this, [&](){
        current_generate_string_type_ = GenerateStringType::BirthDateTime;
        ui->leString->setReadOnly(true);

        _updateWaterMarkedPicture();
    });

    connect(ui->rbModifyTime, &QRadioButton::clicked, this, [&](){
        current_generate_string_type_ = GenerateStringType::ModifyDateTime;
        ui->leString->setReadOnly(true);

        _updateWaterMarkedPicture();
    });

    connect(ui->rbManualInput, &QRadioButton::clicked, this, [&](){
        current_generate_string_type_ = GenerateStringType::MaunalInput;
        ui->leString->setReadOnly(false);

        _updateWaterMarkedPicture();
    });

    connect(ui->rbDigitalFont, &QRadioButton::clicked, this, [&](){
        ui->cbDigitalFonts->setEnabled(true);
        ui->fcbSystemFonts->setEnabled(false);

        _updateWaterMarkedPicture();
    });
    connect(ui->rbSystemFont, &QRadioButton::clicked, this, [&](){
        ui->cbDigitalFonts->setEnabled(false);
        ui->fcbSystemFonts->setEnabled(true);

        _updateWaterMarkedPicture();
    });

    connect(ui->cbDigitalFonts, &QComboBox::currentTextChanged, this, &MainWindow::_updateWaterMarkedPicture);
    connect(ui->fcbSystemFonts, &QFontComboBox::currentTextChanged, this, &MainWindow::_updateWaterMarkedPicture);

    connect(ui->dsbFontPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::_updateWaterMarkedPicture);
    connect(ui->dsbBottomPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::_updateWaterMarkedPicture);
    connect(ui->dsbRightPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::_updateWaterMarkedPicture);

    _reset();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pbLoadFile_clicked()
{
    QString filename = ui->leLoadFilename->text();

    // 检查文件名合法性
    QFileInfo fileinfo(filename);
    if (filename.isEmpty() || filename.isNull() || !fileinfo.exists()) {
        QMessageBox::warning(this, tr("错误"),
                             tr("文件名错误或不存在"));
        return;
    }

    try {
        // 图片读取尝试，如果图片读取失败或不是图片文件会抛出构造函数异常
        mark_generator_ = WaterMarkGenerator(filename);
        if( !mark_generator_.isNull() ) {
            QMessageBox::information(this, tr("通知"),
                                     tr("载入文件成功"));
        }

        current_state_ = State::FileLoaded;
        loaded_filename_ = filename;
    } catch (...) {
        QMessageBox::warning(this, tr("错误"),
                             tr("载入文件错误"));
        _reset();
        return;
    }

    // 更新一次水印
    if (!_updateWaterMarkedPicture()) {
        QMessageBox::warning(this, tr("错误"),
                             error_msg_);
        return;
    }
}

void MainWindow::on_pbSaveFile_clicked()
{
    if (current_state_ == State::FileUnloaded) {
        QMessageBox::warning(this, tr("错误"), tr("保存文件失败, 未载入文件"));
        return;
    }

    if (current_display_img_.isNull()) {
        QMessageBox::warning(this, tr("错误"), tr("保存文件失败, 当前显示图片"));
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

    if (!current_display_img_.save(filename, nullptr, 100)) {
        QMessageBox::warning(this, tr("错误"), tr("保存图片失败"));
        return;
    }
}

void MainWindow::on_pbChooseFile_clicked()
{
    static QString last_dir;
    QString filename = QFileDialog::getOpenFileName(this, tr("请选择图片"), last_dir, tr("Images (*.png *.jpg);; All Files(*)"));
    if (filename.isEmpty() || filename.isNull())
    {
        return;
    }

    QFileInfo fileinfo(filename);
    last_dir = fileinfo.absolutePath();

    ui->leLoadFilename->setText(filename);

    on_pbLoadFile_clicked();
}

void MainWindow::_updateGenerateStringText()
{
    Q_ASSERT(current_state_ != State::FileUnloaded);
    Q_ASSERT(!loaded_filename_.isNull() && !loaded_filename_.isEmpty());

    const auto& filename = loaded_filename_;

    // assert the filename should exist
    QFileInfo fileinfo(filename);
    Q_ASSERT(fileinfo.exists());

    QString generated_str;
    switch (current_generate_string_type_) {
    case GenerateStringType::ShootDateTime:
        generated_str = PictureDateExtracter::getShootDateTimeStringOfPicture(filename);
        break;
    case GenerateStringType::BirthDateTime:
        generated_str = PictureDateExtracter::getBirthTimeStringOfFile(filename);
        break;
    case GenerateStringType::ModifyDateTime:
        generated_str = PictureDateExtracter::getLastModifyTimeStringOfFile(filename);
        break;
    case GenerateStringType::MaunalInput:
    default:
        return;
    }

    qDebug() << generated_str << (int)current_generate_string_type_;
    ui->leString->setText(generated_str);
    return;
}

bool MainWindow::_updateWaterMarkedPicture()
{
    switch (current_state_) {
    case State::FileUnloaded:
        RETURN_ERROR(tr("生成水印失败, 未载入文件"));
    case State::FileLoaded:
        break; // continue steps outside switch
    default:
        RETURN_ERROR(tr("未知状态错误, 状态%0").arg(static_cast<int>(current_state_)));
    }

    if (mark_generator_.isNull()) {
        RETURN_ERROR(tr("生成水印失败, 图片文件读取错误"));
    }

    /* 更新字符串文本框 */
    _updateGenerateStringText();

    /* 选择字体 */
    if (ui->rbDigitalFont->isChecked()) {
        if (ui->cbDigitalFonts->currentIndex() == 0) {
            /* traditional font */
            mark_generator_.setFontFromFile(":/font/MTC-7-Segment.ttf");
        } else {
            /* enhanced font */
            mark_generator_.setFontFromFile(":/font/LCD2U-4.ttf");
        }
    } else { /* ui->rbSystemFont->isChecked() */
        mark_generator_.setFont(ui->fcbSystemFonts->currentFont());
    }

    mark_generator_.setPosSize({ui->dsbBottomPercent->value(),
                               ui->dsbRightPercent->value(),
                               ui->dsbFontPercent->value()});
    current_display_img_ = mark_generator_.getWaterMarkedImage(ui->leString->text());

    if (current_display_img_.isNull()) {
        RETURN_ERROR(tr("生成水印失败, 图片文件错误或绘图错误"));
    }

    ui->lbPicture->setPixmap(QPixmap::fromImage(current_display_img_).scaled(ui->lbPicture->size(),
                                                                             Qt::KeepAspectRatio,
                                                                             Qt::SmoothTransformation));

    return true;
}

void MainWindow::_reset()
{
    current_state_ = State::FileUnloaded;

    current_cached_img_ = QImage();
    current_display_img_ = QImage();

    current_generate_string_type_ = GenerateStringType::ModifyDateTime;
    ui->rbModifyTime->click();
    ui->leString->clear();


    ui->lbPicture->clear();
    ui->lbPicture->setText(tr("请载入图片"));
}


void MainWindow::on_pbConfirmManualString_clicked()
{
    if (!_updateWaterMarkedPicture()) {
        QMessageBox::warning(this, tr("错误"), error_msg_);
        return;
    }
}

void MainWindow::on_pbCacheCurrentMark_clicked()
{
    current_cached_img_ = current_display_img_;
    mark_generator_.setImage(current_cached_img_);

    ui->dsbBottomPercent->setValue(ui->dsbBottomPercent->value() + 5.0);
    if (!_updateWaterMarkedPicture()) {
        QMessageBox::warning(this, tr("错误"), error_msg_);
        return;
    }
}
