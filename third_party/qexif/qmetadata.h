#ifndef QMETADATA_H
#define QMETADATA_H

#include <QObject>
#include <QTextEdit>
#include <QDataStream>
#include "qexifimageheader.h"

class QMetaData : public QObject
{
    Q_OBJECT
public:

    enum JpegSectionId {
        M_NONE  = 0x00,
        M_SOF0  = 0xC0,          // Start Of Frame N
        M_SOF1  = 0xC1,          // N indicates which compression process
        M_SOF2  = 0xC2,          // Only SOF0-SOF2 are now in common use
        M_SOF3  = 0xC3,
        M_SOF5  = 0xC5,          // NB: codes C4 and CC are NOT SOF markers
        M_SOF6  = 0xC6,
        M_SOF7  = 0xC7,
        M_SOF9  = 0xC9,
        M_SOF10 = 0xCA,
        M_SOF11 = 0xCB,
        M_SOF13 = 0xCD,
        M_SOF14 = 0xCE,
        M_SOF15 = 0xCF,
        M_SOI   = 0xD8,          // Start Of Image (beginning of datastream)
        M_EOI   = 0xD9,          // End Of Image (end of datastream)
        M_SOS   = 0xDA,          // Start Of Scan (begins compressed data)
        M_JFIF  = 0xE0,          // Jfif marker
        M_EXIF  = 0xE1,          // Exif marker.  Also used for XMP data!
        //    M_XMP   = 0xE1,        // Not a real tag (same value in file as Exif!)
        M_APP2  = 0xE2,
        M_APP3  = 0xE3,          // Meta Marker ( Format like Exif )
        M_APP4  = 0xE4,
        M_APP5  = 0xE5,
        M_APP6  = 0xE6,
        M_APP7  = 0xE7,
        M_APP8  = 0xE8,
        M_APP9  = 0xE9,
        M_APP10  = 0xEA,
        M_APP11  = 0xEB,
        M_APP12  = 0xEC,
        M_IPTC  = 0xED,           // IPTC marker
        M_APP14  = 0xEE,          // Oft für Copyright-Einträge
        M_APP15  = 0xEF,
        M_COM   = 0xFE,           // COMment
        M_DQT   = 0xDB,
        M_DHT   = 0xC4,
        M_DRI   = 0xDD,

        M_FF = 0xFF
    };

    struct JpegRawSection {
        QMetaData::JpegSectionId id;
        QByteArray data;
        bool hasLoaded;
        quint16 length;
    };

    explicit QMetaData(QString jpegFileName,QObject *parent = 0, QTextEdit *messageBox = 0);
    ~QMetaData();
    void clear(void);

    bool loadFromJpeg(QString jpegFileName);
    bool loadFromJpeg(QIODevice *device,bool readCompleteFile = false);
    bool saveToJpeg(QString jpegFileName = "");
    bool saveToJpeg(QIODevice *device);

    QString keyWords(void);
    QStringList keyWordList(void);
    void setKeyWords(QString text);
    void addKeyWords(QString text);
    void setSeparator(QString separator);
    void setKeyWordCodec(QString codecName);

    QList<QExifImageHeader *> *exifImageHeaderList(void);

    static QString sectionIdToText(QMetaData::JpegSectionId id);
    static QString exifValueToText(QExifValue exifValue,QSysInfo::Endian byteOrder = QSysInfo::LittleEndian);
    static QString tagToText(QExifImageHeader::ImageTag tag);
    static QString tagToText(QExifImageHeader::ExifExtendedTag tag);
    static QString tagToText(QExifImageHeader::GpsTag tag);

signals:
    
public slots:
protected:
    void message(QString text, bool clearText = false);
    void bMessage(QByteArray bText, int size = 8);

private:
    QString readExifImageHeader(QExifImageHeader *exifHeader);
    int findExifSection(quint8 id);

    bool loadExif(JpegRawSection *section,quint8 id);
    bool loadComment(JpegRawSection *section);
    //bool loadExifData(QIODevice *device, QDataStream::ByteOrder byteOrder);
    QString toHex(int value,int precision);
    QTextEdit *tEMessage;
    QString fileName;
    QList <JpegRawSection> jpegSections;
    QList<QExifImageHeader *> exifImageHeaders;
    QStringList mKeyWords;
    bool mKeyWordModified;
    QString mKeyWordSeparator;
    QString mKeyWordCodec;
};

#endif // QMETADATA_H
