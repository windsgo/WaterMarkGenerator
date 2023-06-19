#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QImage>
#include <QPixmap>
#include <QDebug>

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    btn_group_ = new QButtonGroup(this);
    btn_group_->addButton(ui->rbShootTime, 0);
    btn_group_->addButton(ui->rbBirthTime, 1);
    btn_group_->addButton(ui->rbModifyTime, 2);

    connect(ui->rbShootTime, &QRadioButton::clicked, this, [&](){
        QString filename = ui->leLoadFilename->text();
        ui->leString->setText(PictureDateExtracter::getShootDateTimeStringOfPicture(filename));
    });

    connect(ui->rbBirthTime, &QRadioButton::clicked, this, [&](){
        QString filename = ui->leLoadFilename->text();
        ui->leString->setText(PictureDateExtracter::getBirthTimeStringOfFile(filename));
    });

    connect(ui->rbModifyTime, &QRadioButton::clicked, this, [&](){
        QString filename = ui->leLoadFilename->text();
        ui->leString->setText(PictureDateExtracter::getLastModifyTimeStringOfFile(filename));
    });

    connect(ui->dsbFontPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::on_pbConfirmSetting_clicked);
    connect(ui->dsbBottomPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::on_pbConfirmSetting_clicked);
    connect(ui->dsbRightPercent, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::on_pbConfirmSetting_clicked);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pbConfirmSetting_clicked()
{
    if (wmg_.isNull()) {
        QMessageBox::warning(this, tr("生成水印失败"), tr("生成水印失败, 未载入文件"));
        return;
    }

    wmg_.setPosSize({ui->dsbBottomPercent->value(), ui->dsbRightPercent->value(), ui->dsbFontPercent->value()});
    QImage img = wmg_.getWaterMarkedImage(ui->leString->text());

    QPixmap pixmap = QPixmap::fromImage(img).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    ui->label->setPixmap(pixmap);

}

void MainWindow::on_pbLoadFile_clicked()
{
    QString filename = ui->leLoadFilename->text();
    try {
        wmg_ = WaterMarkGenerator(filename);
        if( !wmg_.isNull() ) {
            QMessageBox::information(this, tr("载入文件成功"),
                                     tr("载入文件成功"));
        }

        emit ui->rbShootTime->clicked();
    } catch (...) {
        QMessageBox::warning(this, tr("载入文件错误"),
                             tr("载入文件错误"));
        return;
    }

    on_pbConfirmSetting_clicked();
}

void MainWindow::on_pbSaveFile_clicked()
{
    if (wmg_.isNull()) {
        QMessageBox::warning(this, tr("生成水印失败"), tr("生成水印失败, 未载入文件"));
        return;
    }

    QFileInfo input_fileinfo(ui->leLoadFilename->text());
    QString dir = input_fileinfo.absolutePath();
    QString filename = QFileDialog::getSaveFileName(this, tr("请选择保存图片"), dir, tr("Images (*.jpg);; All Files(*)"));
    if (filename.isEmpty() || filename.isNull())
    {
        return;
    }

    if (!wmg_.saveWaterMarkedImage(ui->leString->text(), filename, nullptr)) {
        QMessageBox::warning(this, tr("保存图片失败"), tr("保存图片失败"));
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
//    qDebug() << filename << fileinfo.absolutePath();

    ui->leLoadFilename->setText(filename);

    on_pbLoadFile_clicked();
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    ui->leString->setReadOnly(!checked);

    ui->widgetrB->setEnabled(!checked);
}
