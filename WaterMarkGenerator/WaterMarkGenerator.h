#ifndef WATERMARKGENERATOR_H
#define WATERMARKGENERATOR_H

#include <QImage>
#include <QColor>
#include <QFont>

#include <exception>
#include <string>
class WaterMarkGeneratorException : std::exception
{
protected:
    std::string str_;
public:
    WaterMarkGeneratorException(const std::string& str) : str_(str) {}
    WaterMarkGeneratorException(const QString& str) : str_(str.toStdString()) {}

    const char* what() const noexcept override {
        return str_.c_str();
    }
};

class WaterMarkGenerator
{
public:
    WaterMarkGenerator();
    explicit WaterMarkGenerator(const QString& pic_path);
    explicit WaterMarkGenerator(const QImage& q_image);

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
private:
    template<typename T>
    static inline T restricte_range(const T& num, const T& min, const T& max) {
        if (num < min) { return min; }
        else if (num > max) { return max; }
        else { return num; }
    }

    void _default_init();

public:
    class PosSizeDisc {
        friend class WaterMarkGenerator;
        double from_bottom_percent_ = 0.0;      // 0~80 , [1] / [3]
        double from_right_percent_ = 0.0;       // 0~80 , [2] / [4]
        double font_percent_ = 1.0;             // 0~10 , [5] / [3]

        QPoint getStartDrawPoint(const QSize& img_size,
                                 const QString& display_str,
                                 QFont& font) const;
    public:
        PosSizeDisc() = default;
        PosSizeDisc(double from_bottom_percent,
                    double from_right_percent,
                    double font_percent)
            : from_bottom_percent_(restricte_range(from_bottom_percent, 0.0, 80.0))
            , from_right_percent_(restricte_range(from_right_percent, 0.0, 80.0))
            , font_percent_(restricte_range(font_percent, 0.0, 10.0))
        {}
    };

    /* return if the image is null */
    bool isNull() const { return img_.isNull(); }

    /* set image */
    void setImage(const QImage& q_image) { img_ = q_image; }
    void setImage(const QString& filename) { img_ = QImage(filename); }

    /* params below we have default values, which may be pretty good for common pictures */

    /* set font from given file */
    bool setFontFromFile(const QString& font_path);
    /* set font by QFont */
    inline void setFont(const QFont& font) { font_ = font; }

    /* set str color */
    inline void setColor(const QColor& color) { color_ = color; }

    /* set the position and size of the water mark string */
    inline void setPosSize(const PosSizeDisc& psd) { psd_ = psd; }

    /* get a water-marked image, use given `mark_str` */
    /* if image is null, this will do nothing but return a null `QImage` */
    QImage getWaterMarkedImage(const QString& mark_str) const;

    /* save the water-marked image to `filename`, use given `mark_str` */
    /* if image is null, this will do nothing but return false */
    bool saveWaterMarkedImage(const QString& mark_str,
                              const QString& filename, const char* format = nullptr) const;

private:
    QFont font_;
    QColor color_;
    QImage img_;
    PosSizeDisc psd_;

signals:

};

#endif // WATERMARKGENERATOR_H
