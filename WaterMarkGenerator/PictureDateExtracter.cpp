#include "PictureDateExtracter.h"


#include "third_party/qexifimageheader/qexifimageheader.h"
#include <QFileInfo>
#include <QDateTime>

static const char* format = "'`'yy MM dd";

QString PictureDateExtracter::getShootDateTimeStringOfPicture(const QString &filename)
{
    QExifImageHeader exif;

    if (!exif.loadFromJpeg(filename)) {
        return QString();
    }

    if (!exif.contains(QExifImageHeader::DateTime)) {
        return QString();
    }

    return exif.value(QExifImageHeader::DateTime).toDateTime().date().toString(format);
}

QString PictureDateExtracter::getBirthTimeStringOfFile(const QString &filename)
{
    QFileInfo fileinfo(filename);
    if (!fileinfo.exists()) {
        return QString();
    }

    return fileinfo.birthTime().date().toString(format);
}

QString PictureDateExtracter::getLastModifyTimeStringOfFile(const QString &filename)
{
    QFileInfo fileinfo(filename);
    if (!fileinfo.exists()) {
        return QString();
    }

    return fileinfo.lastModified().date().toString(format);
}
