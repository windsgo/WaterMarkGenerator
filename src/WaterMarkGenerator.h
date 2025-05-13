#ifndef WATERMARKGENERATOR_H
#define WATERMARKGENERATOR_H

#include <QObject>
#include <QSize>
#include <QFont>
#include <QString>
#include <QPoint>
#include <QImage>
#include <QSet>

#include "WaterMarkDesc.h"

namespace wmg {

class WaterMarkGenerator
{
private:
    WaterMarkGenerator() = delete;

public:
    static std::tuple<QPoint, QFont, QRect> getStartDrawPointAndAdjustedFont(const QSize& img_size,
                                                                             const wmg::WaterMarkDesc& wm_desc);

    static QImage getWaterMarkedImage(const QImage& ori_image, const QList<WaterMarkDesc>& desc_list, const QSet<int>& with_border_index_set);
};

} // namespace wmg

#endif // WATERMARKGENERATOR_H
