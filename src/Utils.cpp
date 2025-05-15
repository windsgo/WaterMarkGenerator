#include "Utils.h"

#include <QFontDatabase>
#include <QList>
#include <QFileInfo>
#include <QDateTime>

static constexpr const char* s_date_format = "'`'yy MM dd";
static constexpr const char* s_exifdate_format = "yyyy:MM:dd hh:mm:ss";

namespace wmg {

std::optional<QFont> Utils::getFontFromFile(const QString &font_path)
{
    int fontId = QFontDatabase::addApplicationFont(font_path);
    if (fontId < 0) {
        return std::nullopt;
    }

    QString msyh = QFontDatabase::applicationFontFamilies(fontId).at(0);
    return QFont(msyh);
}

#ifdef USE_QEXIF_LIB
std::optional<QString> Utils::getShootDateTimeStringOfPicture(const QString &filename)
{
    QString date_qstr;
    auto v1_opt = getExifDataOfPictureByKey(filename, QExifImageHeader::ExifExtendedTag::DateTimeOriginal);
    if (v1_opt) {
        // found
        date_qstr = v1_opt->toString();
    } else {
        // not found, try another
        auto v2_opt = getExifDataOfPictureByKey(filename, QExifImageHeader::ExifExtendedTag::DateTimeDigitized);
        if (v2_opt) {
            date_qstr = v2_opt->toString();
        } else {
            // still not found
            return std::nullopt;
        }
    }

    auto datetime = QDateTime::fromString(date_qstr, s_exifdate_format);
    if (datetime.isNull()) {
        qWarning() << "Get ExifDateTime error: convert to QDateTime failed: " << date_qstr;
        return std::nullopt;
    }

    return datetime.toString(s_date_format);
}

std::optional<QExifURational> Utils::getExifDPIResolutionOfPicture(const QString &filename)
{
    auto v_opt = getExifDataOfPictureByKey(filename, QExifImageHeader::ImageTag::XResolution);
    if (!v_opt) {
        return std::nullopt;
    }

    return v_opt->toRational();
}

std::optional<quint16> Utils::getExifDPIUnitOfPicture(const QString &filename)
{
    auto v_opt = getExifDataOfPictureByKey(filename, QExifImageHeader::ImageTag::ResolutionUnit);
    if (!v_opt) {
        return std::nullopt;
    }

    return v_opt->toShort();
}

#else // !USE_QEXIF_LIB
std::optional<Exiv2::ExifData> Utils::getExifDataOfPicture(const QString &filename)
{
    QFileInfo fileinfo(filename);
    if (!fileinfo.exists()) {
        return std::nullopt;
    }

    auto filename_stdstr = filename.toLocal8Bit().toStdString(); // support chinese name folder and pics

    try {
        auto image = Exiv2::ImageFactory::open(filename_stdstr);
        if (!image) {
            qWarning() << "Exiv2::ImageFactory::open failed: !image";
            return std::nullopt;
        }

        if (!image.get()) {
            qWarning() << "Exiv2::ImageFactory::open failed: !image.get()";
            return std::nullopt;
        }

        if (!image->good()) {
            qWarning() << "Exiv2::ImageFactory::open failed: !image->good()";
            return std::nullopt;
        }

        image->readMetadata();
        return image->exifData();

    } catch (const Exiv2::Error& e) {
        qWarning() << "Exiv2::Error: " << e.what();
        return std::nullopt;
    }

    // won't reach here
    return std::nullopt;
}

std::optional<QString> Utils::getShootDateTimeStringFromExifData(const Exiv2::ExifData &exif_data)
{
    auto it = exif_data.findKey(Exiv2::ExifKey{WMG_EXIFDATA_KESTR_DATETIMEORIGINAL});
    if (it == exif_data.end()) {
        // not found
        qWarning() << "Get ExifDateTime warn: first try key " << WMG_EXIFDATA_KESTR_DATETIMEORIGINAL << " not found";
        // try another key
        it = exif_data.findKey(Exiv2::ExifKey{WMG_EXIFDATA_KESTR_DATETIMEDIGITIZED});
        if (it == exif_data.end()) {
            // still not found
            qWarning() << "Get ExifDateTime error: key not found";
            return std::nullopt;
        }

        // found, continue
    }

    // check type
    if (it->typeId() != Exiv2::TypeId::asciiString) {
        qWarning() << "Get ExifDateTime error: not asciiString, typeId: " << it->typeId() << ", typeName: " << it->typeName();
        return std::nullopt;
    }

    auto date_string = it->getValue()->toString();
    qDebug() << "date_string: it->getValue()->toString(): " << date_string;

    auto datetime = QDateTime::fromString(QString::fromStdString(date_string), s_exifdate_format);
    qDebug() << "datetime: " << datetime;

    if (datetime.isNull()) {
        qWarning() << "Get ExifDateTime error: convert to QDateTime failed: " << date_string;
        return std::nullopt;
    }

    return datetime.toString(s_date_format);
}
#endif // USE_QEXIF_LIB

std::optional<QString> Utils::getBirthTimeStringOfFile(const QString &filename)
{
    QFileInfo fileinfo(filename);
    if (!fileinfo.exists()) {
        return std::nullopt;
    }

    return fileinfo.birthTime().date().toString(s_date_format);
}

std::optional<QString> Utils::getLastModifyTimeStringOfFile(const QString &filename)
{
    QFileInfo fileinfo(filename);
    if (!fileinfo.exists()) {
        return std::nullopt;
    }

    return fileinfo.lastModified().date().toString(s_date_format);
}

} // namespace wmg
