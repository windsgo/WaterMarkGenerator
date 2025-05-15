#ifndef UTILS_H
#define UTILS_H

#include <QFont>
#include <QString>
#include <QDebug>
#include <QFileInfo>

#include <optional>

#ifdef USE_QEXIF_LIB
#include "qexifimageheader.h"
#include "qmetadata.h"
#else // !USE_QEXIF_LIB
#include "exiv2/exiv2.hpp"

#define WMG_EXIFDATA_KEYSTR_EXIF_XRESOLUTION R"(Exif.Image.XResolution)" // Rational e.g. (300/1)
#define WMG_EXIFDATA_KEYSTR_EXIF_YRESOLUTION R"(Exif.Image.YResolution)" // Rational
#define WMG_EXIFDATA_KEYSTR_EXIF_RESOLUTIONUNIT R"(Exif.Image.ResolutionUnit)" // Short e.g. (2)

#define WMG_EXIFDATA_KEYSTR_THUMBNAIL_XRESOLUTION R"(Exif.Thumbnail.XResolution)" // Rational
#define WMG_EXIFDATA_KEYSTR_THUMBNAIL_YRESOLUTION R"(Exif.Thumbnail.YResolution)" // Rational
#define WMG_EXIFDATA_KEYSTR_THUMBNAIL_RESOLUTIONUNIT R"(Exif.Thumbnail.ResolutionUnit)" // Short

#define WMG_EXIFDATA_KEYSTR_COPYRIGHT R"(Exif.Image.CopyRight)" // Ascii e.g. (windsgo)
#define WMG_EXIFDATA_KEYSTR_SOFTWARE R"(Exif.Image.Software)" // Ascii e.g. (RICOH GR III Ver. 1.91)
#define WMG_EXIFDATA_KESTR_DATETIMEORIGINAL R"(Exif.Photo.DateTimeOriginal)" // Ascii e.g. (2025:05:11 15:40:27)
#define WMG_EXIFDATA_KESTR_DATETIMEDIGITIZED R"(Exif.Photo.DateTimeDigitized)" // Ascii e.g. (2025:05:11 15:40:27)
#endif // USE_QEXIF_LIB

namespace wmg {

class Utils
{
private:
    Utils() = delete;

public:
    template<typename T>
    static inline T restricte_range(const T& num, const T& min, const T& max) {
        if (num < min) { return min; }
        else if (num > max) { return max; }
        else { return num; }
    }

    static std::optional<QFont> getFontFromFile(const QString& font_path);

#ifdef USE_QEXIF_LIB
    /* Get EXIF Shoot Date */
    static std::optional<QString> getShootDateTimeStringOfPicture(const QString& filename);
    static std::optional<QExifURational> getExifDPIResolutionOfPicture(const QString& filename);
    static std::optional<quint16> getExifDPIUnitOfPicture(const QString& filename);

    template <typename KEY_t>
    static inline std::optional<QExifValue> getExifDataOfPictureByKey(const QString& filename, KEY_t key)
    {
        QFileInfo fileinfo(filename);
        if (!fileinfo.exists()) {
            return std::nullopt;
        }

        auto metadata = QMetaData(filename);
        auto p_exif_header_list = metadata.exifImageHeaderList();
        if (!p_exif_header_list || p_exif_header_list->empty()) {
            qWarning() << "getExifDataOfPictureByKey" << "!p_exif_header_list || p_exif_header_list->empty()";
            return std::nullopt;
        }

        qDebug() << p_exif_header_list->size();

        auto p_exif_header_0 = p_exif_header_list->at(0);
        if (!p_exif_header_0) {
            qWarning() << "getExifDataOfPictureByKey" << "!p_exif_header_0";
            return std::nullopt;
        }

        if (p_exif_header_0->contains(key)) {
            return p_exif_header_0->value(key);
        } else {
            return std::nullopt;
        }
    }

#else // !USE_QEXIF_LIB
    /* Get EXIF Data Map */
    static std::optional<Exiv2::ExifData> getExifDataOfPicture(const QString& filename);

    /* Get EXIF Shoot Date */
    static std::optional<QString> getShootDateTimeStringFromExifData(const Exiv2::ExifData& exif_data);
#endif // USE_QEXIF_LIB

    /* Get Birth Time of File */
    static std::optional<QString> getBirthTimeStringOfFile(const QString& filename);

    /* Get Last Modify Time of File */
    static std::optional<QString> getLastModifyTimeStringOfFile(const QString& filename);
};

} // namespace wmg

#endif // UTILS_H
