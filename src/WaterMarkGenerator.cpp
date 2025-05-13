#include "WaterMarkGenerator.h"

#include <QDebug>
#include <QPainter>
#include <QFontMetrics>

#include <QPainterPath>

namespace wmg {

std::tuple<QPoint, QFont, QRect> WaterMarkGenerator::getStartDrawPointAndAdjustedFont(const QSize &img_size, const WaterMarkDesc &wm_desc)
{
    auto desc = wm_desc; // copy
    desc.validate_self(); // validate

    // desc.font.setPointSize(img_size.height() * desc.font_percent / 100); // will depend on dpi(no)
    desc.font.setPixelSize(img_size.height() * desc.font_percent / 100); // no dpi dependency(yes)

    QFontMetrics fm(desc.font);

    // int char_height = fm.height();
    int char_width = fm.horizontalAdvance(wm_desc.str);

    QPoint start_point;
    start_point.setX(img_size.width() - char_width - desc.from_right_percent * img_size.width() / 100);

    start_point.setY(img_size.height() - desc.from_bottom_percent * img_size.height() / 100);

    auto rect_start_point = start_point;
    rect_start_point.setY(rect_start_point.y() - fm.height());
    QRect rect{rect_start_point, QSize{char_width, fm.height()}}; // test fm.height()

    return {start_point, desc.font, rect};
}

QImage WaterMarkGenerator::getWaterMarkedImage(const QImage &ori_image, const QList<WaterMarkDesc> &desc_list, const QSet<int>& with_border_index_set)
{
    QImage img{ori_image}; // copy

    if (ori_image.isNull()) {
        qWarning() << "ori_image is Null";
        return img;
    }

    QPainter painter(&img);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QPen selecting_text_pen;
    selecting_text_pen.setWidth(3);
    // selecting_text_pen.setColor(Qt::gray);
    selecting_text_pen.setColor(Qt::yellow);

    QPen border_pen;
    border_pen.setWidth(img.width() / 200);
    border_pen.setColor(QColor{255, 149, 21});
    border_pen.setStyle(Qt::DashLine);

    // paint each water-mark
    for (int i = 0; i < desc_list.size(); ++i) {
        const auto& desc = desc_list.at(i);
        auto [point, font, rect] = WaterMarkGenerator::getStartDrawPointAndAdjustedFont(ori_image.size(), desc);

        bool selecting_this = with_border_index_set.contains(i);
        if (selecting_this) {
#if 0
            // paint border
            painter.setPen(border_pen);
            painter.drawRect(rect);
#endif // no need to paint border any more, use QPainterPath

            // set this text pen color
            QPainterPath path;
            path.addText(point, font, desc.str);

            painter.setPen(selecting_text_pen);
            // painter.setBrush(Qt::yellow);
            painter.setBrush(desc.color);

            painter.drawPath(path);
        } else {
            // normal paint text
            painter.setFont(font);
            painter.setPen(desc.color);
            painter.drawText(point, desc.str);
        }


        qDebug() << "painting string: " << desc.str << ", point: " << point << ", rect:" << rect;

    }

    return img;
}


} // namespace wmg
