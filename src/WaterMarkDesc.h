#ifndef WATERMARKDESC_H
#define WATERMARKDESC_H

#include <QFont>
#include <QFontDatabase>
#include <QString>
#include <QColor>

namespace wmg {

/*

      ***************************  ---
      *                         *   |
      *                         *   |
      *                         *   |
      *                         *   |
      *                         *   |
      *                         *   |
      *                         *   |
      *                         *   |
      *                         *
      *                         *  [3]
      *                         *
      *                         *   |
      *                         *   |
      *       ---          |[2]|*   |
      *       [5] 'YY MM DD     *   |
      *       ---               *   |
      *       [1]               *   |
      *       ---               *   |
      *************************** ---
      |---------- [4] ----------|

*/

// Description about how to generate a water-mark in an image
struct WaterMarkDesc {
    static constexpr const double from_bottom_percent_max = 80;
    static constexpr const double from_right_percent_max = 80;
    static constexpr const double font_percent_max = 80;

    double from_bottom_percent {9.50};          // 0~80 , [1] / [3]
    double from_right_percent {17.50};        // 0~80 , [2] / [4]
    double font_percent {3.77};               // 0~10 , [5] / [3]

    QFont font;
    QString str{""};
    // QColor color{"#FF9515"};
    QColor color{255, 149, 21};

    void validate_self();

    WaterMarkDesc();
};

}; // namespace wmg

#endif // WATERMARKDESC_H
