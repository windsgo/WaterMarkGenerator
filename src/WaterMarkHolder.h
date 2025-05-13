#ifndef WATERMARKHOLDER_H
#define WATERMARKHOLDER_H

#include <QObject>
#include <QImage>
#include <QList>

#include <memory>

#include "WaterMarkDesc.h"

namespace wmg {

class WaterMarkHolder
{
public:
    using ptr = std::shared_ptr<WaterMarkHolder>;
    WaterMarkHolder();

    // reset All
    void reset();

    // set image
    void setImage(const QImage& image);

    // is Null?
    bool isNull() const { return img_.isNull(); }

    // get or modify description list
    auto& getDesciptionList() { return desc_list_; }
    const auto& getDesciptionList() const { return desc_list_; }

    // generate water-marked image
    // if `img_` is null, this will return a null `QImage`
    QImage generateWaterMarkedImage(const QSet<int>& with_border_index_set) const;

private:
    QImage img_;
    QList<WaterMarkDesc> desc_list_;
};

} // namespace wmg

#endif // WATERMARKHOLDER_H
