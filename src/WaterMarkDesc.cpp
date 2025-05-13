#include "WaterMarkDesc.h"

#include "Utils.h"
#include <QDebug>

namespace wmg {

void WaterMarkDesc::validate_self() {
    from_bottom_percent = Utils::restricte_range<double>(from_bottom_percent, 0, from_bottom_percent_max);
    from_right_percent = Utils::restricte_range<double>(from_right_percent, 0, from_right_percent_max);
    font_percent = Utils::restricte_range<double>(font_percent, 0, font_percent_max);

    if (str.isNull())
        str = QString{""};
}

WaterMarkDesc::WaterMarkDesc() {
    // load default font from resource
    int fontId = QFontDatabase::addApplicationFont(":/font/MTC-7-Segment.ttf");
    if (fontId >= 0) {
        QString msyh = QFontDatabase::applicationFontFamilies(fontId).at(0);
        font = QFont(msyh, 20, QFont::Weight::Normal);
    } else {
        qWarning() << "load default font failed";
    }
}

}; // namespace wmg
