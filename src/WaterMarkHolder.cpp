#include "WaterMarkHolder.h"

#include "WaterMarkGenerator.h"

namespace wmg {

WaterMarkHolder::WaterMarkHolder() {}

void WaterMarkHolder::reset()
{
    img_ = QImage();
    desc_list_.clear();
}

void WaterMarkHolder::setImage(const QImage &image)
{
    img_ = image;
}

QImage WaterMarkHolder::generateWaterMarkedImage(const QSet<int>& with_border_index_set) const
{
    return WaterMarkGenerator::getWaterMarkedImage(img_, desc_list_, with_border_index_set);
}

} // namespace wmg
