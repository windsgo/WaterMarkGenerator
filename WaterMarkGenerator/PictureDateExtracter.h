#ifndef PICTUREDATEEXTRACTER_H
#define PICTUREDATEEXTRACTER_H

#include <QString>

class PictureDateExtracter
{
private:
    PictureDateExtracter() {};

public:
    /* Get EXIF Shoot Date, return empty `QString()` if failed */
    static QString getShootDateTimeStringOfPicture(const QString& filename);

    /* Get Birth Time of File */
    static QString getBirthTimeStringOfFile(const QString& filename);

    /* Get Last Modify Time of File */
    static QString getLastModifyTimeStringOfFile(const QString& filename);

};

#endif // PICTUREDATEEXTRACTER_H
