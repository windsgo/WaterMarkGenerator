#ifndef UTILS_H
#define UTILS_H

#include <QFont>
#include <QString>

#include <optional>

#include "exiv2/exif.hpp"

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

    /* Get EXIF Data Map */
    static std::optional<Exiv2::ExifData> getExifDataOfPicture(const QString& filename);

    /* Get EXIF Shoot Date */
    static std::optional<QString> getShootDateTimeStringFromExifData(const Exiv2::ExifData& exif_data);

    /* Get Birth Time of File */
    static std::optional<QString> getBirthTimeStringOfFile(const QString& filename);

    /* Get Last Modify Time of File */
    static std::optional<QString> getLastModifyTimeStringOfFile(const QString& filename);
};

} // namespace wmg

#endif // UTILS_H
