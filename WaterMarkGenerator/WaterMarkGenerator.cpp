#include "WaterMarkGenerator.h"
#include <QPainter>
#include <QPen>
#include <QFontDatabase>

static constexpr const char* default_font_path = ":/font/MTC-7-Segment.ttf";

WaterMarkGenerator::WaterMarkGenerator()
{
    _default_init();
}

WaterMarkGenerator::WaterMarkGenerator(const QString &pic_path)
    : img_(pic_path)
{
    if (img_.isNull()) {
        throw WaterMarkGeneratorException("Cannot Open Picture: " + pic_path);
    }
    _default_init();
}

WaterMarkGenerator::WaterMarkGenerator(const QImage &q_image)
    : img_(q_image)
{
    if (img_.isNull()) {
        throw WaterMarkGeneratorException(QString("Invalid q_image input"));
    }
    _default_init();
}

void WaterMarkGenerator::_default_init()
{
    int fontId = QFontDatabase::addApplicationFont(default_font_path);
    if (fontId >= 0) {
        QString msyh = QFontDatabase::applicationFontFamilies(fontId).at(0);
        font_ = QFont(msyh, 20, QFont::Weight::Normal);
    }
    color_ = QColor("#FF9515"); /* default best color */

    psd_ = PosSizeDisc(5.50, 13.50, 1.57); /* default best params */
}

bool WaterMarkGenerator::setFontFromFile(const QString &font_path)
{
    int fontId = QFontDatabase::addApplicationFont(font_path);
    if (fontId < 0) {
        return false;
    }

    QString msyh = QFontDatabase::applicationFontFamilies(fontId).at(0);
    font_ = QFont(msyh);
    return true;
}

QImage WaterMarkGenerator::getWaterMarkedImage(const QString& mark_str) const
{
    if (img_.isNull()) {
        return img_;
    }

    QImage ret(img_);

    QPainter painter(&ret);

    QFont using_font = font_;

    QPoint start_point = psd_.getStartDrawPoint(ret.size(), mark_str, using_font);

    painter.setFont(using_font);
    painter.setPen(color_);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.drawText(start_point, mark_str);

    return ret;
}

bool WaterMarkGenerator::saveWaterMarkedImage(const QString& mark_str,
                                              const QString &filename, const char *format) const
{
    QImage img = getWaterMarkedImage(mark_str);
    if (!img.isNull()) {
        return img.save(filename, format, 100);
    } else {
        return false;
    }
}

QPoint WaterMarkGenerator::PosSizeDisc::getStartDrawPoint(const QSize &img_size,
                                                          const QString &display_str,
                                                          QFont &font) const
{
    font.setPointSize(img_size.height() * font_percent_ / 100);

    QFontMetrics fm(font);

    // int char_height = fm.height();
    int char_width = fm.horizontalAdvance(display_str);

    QPoint start_point;
    start_point.setX(img_size.width() - char_width - from_right_percent_ * img_size.width() / 100);

    start_point.setY(img_size.height() - from_bottom_percent_ * img_size.height() / 100);
    return start_point;
}
