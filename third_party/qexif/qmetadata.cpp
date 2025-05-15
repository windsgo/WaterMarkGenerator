#include "qmetadata.h"
#include <QFile>
#include <QBuffer>
#include <QTextCodec>

QMetaData::QMetaData(QString jpegFileName, QObject *parent, QTextEdit *messageBox) :
    QObject(parent)
{
    mKeyWordSeparator = ' ';
    mKeyWordCodec = "UTF-8";
    tEMessage = messageBox;
    if ( tEMessage ) {
        tEMessage->setLineWrapMode(QTextEdit::NoWrap);
    }
    loadFromJpeg(jpegFileName);
}
QMetaData::~QMetaData() {
    clear();
}

void QMetaData::clear(void){
    jpegSections.clear();
    while (exifImageHeaders.count() ) {
        delete exifImageHeaders.takeLast();
    }
}

// EXIF 	APP1 (0xFFE1) 	"EXIF\x00\x00" or "EXIF\x00\xFF" 	TIFF, EXIF, DCF, TIFF/EP
// XMP      APP1 (0xFFE1) 	"http://ns.adobe.com/xap/1.0/\x00" 	XMP Can also contain data as per DCMI Terms
// META 	APP3 (0xFFE3) 	"META\x00\x00" or "Meta\x00\x00"
bool QMetaData::loadExif(JpegRawSection *section,quint8 id){
    // load Exif or Meta
    // Data = "Exif\x00\x00II......"
    // Data = "Exif\x00\x00MM......"
    // Data = "Meta\x00\x00II......"
    // Data = "Meta\x00\x00MM......"
    QString textId = section->data.left(4);
    if (( textId == "EXIF") || (textId == "META") || ( textId == "Exif") || (textId == "Meta")) {
        QByteArray exifData = section->data.mid(6);
        QExifImageHeader *exifImageHeader = new QExifImageHeader(&exifData,quint8(id));
        //        exifImageHeader->loadFromData(&exifData);
        if ( exifImageHeader->success() )  {
            exifImageHeaders << exifImageHeader;
            message(textId+" data loaded successful");
            message(readExifImageHeader(exifImageHeader));
            return true;
        }
        delete exifImageHeader;
        message("Exif data failed");
    }
    return false;
}
QString QMetaData::keyWords(void){
    return mKeyWords.join(mKeyWordSeparator);
}
QStringList QMetaData::keyWordList(void){
    return mKeyWords;
}

void QMetaData::setKeyWords(QString text){
    mKeyWords.clear();
    mKeyWords.append(text.split(mKeyWordSeparator));
    mKeyWords.removeDuplicates();
    mKeyWordModified = true;
}

void QMetaData::addKeyWords(QString text){
    QStringList list = text.split(mKeyWordSeparator);
    foreach ( QString keyWord, list) {
        if ( mKeyWords.indexOf(keyWord) < 0) {
            mKeyWords.append(keyWord);
            mKeyWordModified = true;
        }
    }
}
void QMetaData::setSeparator(QString separator){
    mKeyWordSeparator = separator;
}
void QMetaData::setKeyWordCodec(QString codecName) {
    mKeyWordCodec = codecName;
}

bool QMetaData::loadComment(JpegRawSection *section){
    int size = section->length - 2;
    QString imageCommentar;
    if ( section->data.left(8) == QByteArray::fromRawData("UNICODE\0",8)) {
        // saved in exif style ??
        QTextCodec *codec = QTextCodec::codecForName("UTF-16");
        imageCommentar = codec->toUnicode(section->data.mid(8));
        size -= 8;
        size /=2;
    } else {
        //  saved as UTF-8
        QTextCodec *codec = QTextCodec::codecForName(mKeyWordCodec.toLatin1());
        imageCommentar = codec->toUnicode(section->data);
        size = imageCommentar.size();
    }
    setKeyWords(imageCommentar);
    mKeyWordModified = false;
    if ( imageCommentar.size() == size ) {
        message("Comment data loaded successful");
        message("  "+imageCommentar);
        return true;
    }
    return false;
}
bool QMetaData::saveToJpeg(QString jpegFileName){
    int pos = jpegFileName.lastIndexOf("/");
    message(jpegFileName.mid(pos+1),true);

    // bool loadCompleted{true};
    // QFile file(fileName);
    // if(file.open(QIODevice::ReadOnly)) {
    //     loadCompleted = loadFromJpeg(&file,true);
    //     file.close();
    // }
    if ( jpegFileName.size() ) {
        fileName = jpegFileName;
    }
    // if ( loadCompleted) {
        QFile file(fileName);
        if(file.open(QIODevice::ReadWrite)) {
            bool result = saveToJpeg(&file);
            if ( result ) {
                file.flush();
            }
            file.close();
            return result;
        }
        message("Write file: Error "+QString::number(file.error()));
    // } else {
    //     message("Read complete file: Error "+QString::number(file.error()));
    // }
    return false;
}
int QMetaData::findExifSection(quint8 id) {
    for(int index = 0; index < jpegSections.count(); index++) {
        if (jpegSections[index].id == id) {
            return index;
        }
    }
    return -1;
}

bool QMetaData::saveToJpeg(QIODevice *device){

    if( device->isSequential() ) {
        return false;
    }
    foreach( QExifImageHeader *exifHeader,exifImageHeaders) {
        if ( exifHeader->modified()) {
            QByteArray exifSection = "";
            QBuffer buffer( &exifSection );
            if( !buffer.open( QIODevice::WriteOnly ) )
                return false;
            exifHeader->write( &buffer );
            buffer.close();

            quint8 id = exifHeader->headerId();
            int index = findExifSection(id);
            if ( index > -1 ) {
                jpegSections[index].data = jpegSections[index].data.left(6)+exifSection;
                jpegSections[index].length = jpegSections[index].data.size()+2;

            } else {
                // TODO or NOT TODO ??? create
            }
        }
    }
    if ( mKeyWordModified ) {
        int index = findExifSection(M_COM);
        if ( index < 0 ) {
            JpegRawSection jpegSection;
            jpegSection.id = M_COM;
            index = findExifSection(M_EXIF)+1;
            jpegSections.insert(index,jpegSection);
        }
        QTextCodec *codec = QTextCodec::codecForName(mKeyWordCodec.toLatin1());
        jpegSections[index].data = codec->fromUnicode(keyWords());
        jpegSections[index].length = jpegSections[index].data.size()+2;
    }
    bool isFirst = true;
    foreach( JpegRawSection jpegSection,jpegSections) {
        QByteArray exif = "";
        QDataStream stream( &exif,QIODevice::WriteOnly );
        stream.setByteOrder( QDataStream::BigEndian );
        if ( isFirst ) {
            stream << quint16( 0xFFD8 ); // SOI
            isFirst = false;
        }
        stream << quint8( 0xFF );
        stream << quint8( jpegSection.id );
        if ( jpegSection.length ) {
            stream << jpegSection.length;
        }
        device->write( exif );
        device->write( jpegSection.data );
    }
    return true;
}


bool QMetaData::loadFromJpeg(QString jpegFileName){
    fileName = jpegFileName;

    int pos = jpegFileName.lastIndexOf("/");
    message(jpegFileName.mid(pos+1),true);
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly)) {
        bool result = loadFromJpeg(&file);
        file.close();
        return result;
    }
    message("Open file: Error "+QString::number(file.error()));
    return false;
}
bool QMetaData::loadFromJpeg(QIODevice *device,bool readRawData){
    //Die Längenangaben der Segmente enthalten übrigens auch den Platz für die Längenangaben selbst:
    //Ein leeres Segment hat daher die Länge 2, da dies der Länge der Komponenten s1 und s2 selbst entspricht.

    //Auf das Start-of-Scan-(SOS)-Segment (Marker FF DA) folgen direkt die komprimierten Daten,
    //bis zum Start des nächsten Segments, das durch den nächsten Marker angezeigt wird.
    //Sollte innerhalb der Daten ein FF auftreten, so wird dies mit einer folgenden 00 (Null) markiert.
    //Andere Werte zeigen das Auftreten eines neuen Segments bzw. Markers an.
    //Ausnahme: Folgt dem FF einer der Restart-Marker (D0 - D7) so setzen sich die Daten direkt dahinter weiter fort: FF DA ... daten ... FF D0 ... daten ...
    if ( readRawData ) {
        jpegSections.clear();
    } else {
        clear();
    }
    QDataStream stream( device );
    stream.setByteOrder( QDataStream::BigEndian );

    bool result = false;
    quint8 marker = 0;
    quint8 sectionId = 0;
    if( device->read( 2 ) != "\xFF\xD8" )
        return false;
    while ( !device->atEnd() ) {
        while( !(( marker == M_FF) && (sectionId != M_FF)) ) {
            if( device->atEnd() ) {
                return result;
            }
            marker = sectionId;
            stream >> sectionId;
        }
        marker = 0;
        result = true;

        JpegRawSection section;
        section.hasLoaded = false;
        section.id = static_cast<JpegSectionId>(sectionId);
        message("<b>"+sectionIdToText(section.id)+"</b>");
        if ( (sectionId == M_SOS) ) {  // should be the last section id
            section.data = "";
            section.length = 0;
            if ( !readRawData ) {
                return true;
            }
        } else {
            stream >> section.length;
        }

        if ( readRawData ) {
            if ( (sectionId == M_SOS) ) {  // should be the last section id
                section.data = device->readAll();
            } else {
                section.data = device->read( section.length - 2 );
            }
            jpegSections << section;
            message("    length="+QString::number(section.data.size()+2));
        } else {
            switch ( sectionId ) {
            case M_NONE: continue; break;

            case M_EXIF:
                section.data = device->read( section.length - 2 );
                section.hasLoaded = loadExif(&section,sectionId);
                //bMessage(section.data.mid(6),8);
                if ( !section.hasLoaded ) {       // Exif marker.  Also used for XMP data!
                    message("M_XMP");
                }
                break;
            case M_APP3:         // one more EXIF section but named "Meta" found at KODAK images
                section.data = device->read( section.length - 2 );
                section.hasLoaded = loadExif(&section,sectionId);
                //bMessage(section.data.mid(6),8);
                break;
            case M_COM:          // Comment
                section.data = device->read( section.length - 2 );
                if ( !readRawData ) {
                    section.hasLoaded = loadComment(&section);
                }
                break;

            case M_SOF0:         // Start Of Frame N
            case M_SOF1:         // N indicates which compression process
            case M_SOF2:         // Only SOF0-SOF2 are now in common use
            case M_SOF3:
            case M_SOF5:         // NB: codes C4 and CC are NOT SOF markers
            case M_SOF6:
            case M_SOF7:
            case M_SOF9:
            case M_SOF10:
            case M_SOF11:
            case M_SOF13:
            case M_SOF14:
            case M_SOF15:
            case M_SOI:          // Start Of Image (beginning of datastream)
            case M_EOI:          // End Of Image (end of datastream)
            case M_SOS:          // Start Of Scan (begins compressed data)
            case M_JFIF:         // Jfif marker
            case M_APP2:
            case M_APP4:
            case M_APP5:
            case M_APP6:
            case M_APP7:
            case M_APP8:
            case M_APP9:
            case M_APP10:
            case M_APP11:
            case M_APP12:
            case M_IPTC:         // IPTC marker
            case M_APP14:        // Oft für Copyright-Einträge
            case M_APP15:
            case M_DQT:
            case M_DHT:
            case M_DRI:
            default:  break;
            }
            if ( !section.hasLoaded ) {
                device->seek( device->pos() + section.length - 2 );
                message("    length="+QString::number(section.length));
                //bMessage(section.data,8);
            }
        }
    }
    return result;
}

void QMetaData::message(QString text, bool clearText){
    if ( tEMessage) {
        if ( clearText ) {
            tEMessage->clear();
        }
        tEMessage->append(text);
    }
}
QString QMetaData::toHex(int value,int precision){
    QString text = QString::number(value,16);
    text = text.rightJustified(precision,'0');
    int pos = 4;
    while ( text.size() > pos) {
        text.insert(text.size()-pos,':');
        pos += 5;
    }
    return text;
}

void QMetaData::bMessage(QByteArray bText, int size){
    if ( tEMessage ) {
        QFont oldFont = tEMessage->font();
        QFont newFont = oldFont;
        newFont.setFamily("Monospace");
        tEMessage->setCurrentFont(newFont);

        QColor color = tEMessage->textColor();
        tEMessage->setTextColor(Qt::blue);

        QString hexText = "";
        QString text = "";
        bool skipped = false;
        QString hexAdr = toHex(0,8);
        for ( int count = 0; count < bText.size(); count++) {
            quint8 zeichen = bText[count];
            hexText += toHex(zeichen,2)+" ";
            if ( zeichen < 32 ) {
                text += ".";
            } else {
                char c = zeichen;
                text += c;
            }
            if ( ((count+1) % size) == 0 ) {
                message("  "+hexAdr+"  "+hexText+text);
                hexAdr = toHex(count+1,8);
                hexText = "";
                text = "";
                //count = 0;
            }
            if ( !skipped) {
                if ( count > 1000 ) {
                    skipped = true;
                    if ( bText.size() > 2000 ) {
                        message(".... much more data");
                        count = bText.size() - 1000;
                        while( ((count+1) % size) ) count++;
                        hexAdr = toHex(count+1,8);
                        hexText = "";
                        text = "";
                    }
                }
            }
        }
        tEMessage->setTextColor(color);
        tEMessage->setCurrentFont(oldFont);
    }
}

QList<QExifImageHeader *> *QMetaData::exifImageHeaderList(void){
    return &exifImageHeaders;
}

QString QMetaData::readExifImageHeader(QExifImageHeader *exifHeader){
    QString exifInfo = "";
    if ( exifHeader->imageTags().count()) {
        exifInfo += "Image tags<br />";
        foreach (QExifImageHeader::ImageTag tag,exifHeader->imageTags() ) {
            exifInfo += "  <b>"+tagToText(tag)+": </b>"+exifValueToText(exifHeader->value(tag),exifHeader->byteOrder())+"<br />";
        }
    }
    if ( exifHeader->extendedTags().count()) {
        exifInfo += "Extended tags<br />";
        foreach (QExifImageHeader::ExifExtendedTag tag,exifHeader->extendedTags() ) {
            QExifValue value = exifHeader->value(tag);
            exifInfo += "  <b>"+tagToText(tag)+": </b>"+exifValueToText(value)+"<br />";
            //exifInfo += "  <b>"+tagToText(tag)+": </b>"+exifValueToText(exifHeader->value(tag))+"<br />";
        }
    }
    if ( exifHeader->gpsTags().count()) {
        exifInfo += "GPS tags<br />";
        foreach (QExifImageHeader::GpsTag tag,exifHeader->gpsTags() ) {
            exifInfo += "  <b>"+tagToText(tag)+": </b>"+exifValueToText(exifHeader->value(tag))+"<br />";
        }
    }
    return exifInfo;
}

QString QMetaData::exifValueToText(QExifValue exifValue,QSysInfo::Endian byteOrder){
    switch (exifValue.type()) {
    case QExifValue::Byte: return QString::number(exifValue.toByte());
    case QExifValue::Ascii: return exifValue.toString(byteOrder);
    case QExifValue::Short: return QString::number(exifValue.toShort());
    case QExifValue::Long: return QString::number(exifValue.toLong());
    case QExifValue::Rational: return QString::number(exifValue.toRational().first)+"/"+QString::number(exifValue.toRational().second);
    case QExifValue::SignedLong: return QString::number(exifValue.toSignedLong());
    case QExifValue::SignedRational: return QString::number(exifValue.toRational().first)+"/"+QString::number(exifValue.toRational().second);
    case QExifValue::Undefined: {
//        return "undef";//exifValue.toString(byteOrder);
        QByteArray value = exifValue.toByteArray();
        return QString(value);
    }
    }
    return "ERROR";
}

QString QMetaData::sectionIdToText(JpegSectionId id){
    switch (id) {
    case M_NONE:  return ("M_NO_ID");
    case M_SOF0:  return ("M_SOF0");           // Start Of Frame N
    case M_SOF1:  return ("M_SOF1");             // N indicates which compression process
    case M_SOF2:  return ("M_SOF2");             // Only SOF0-SOF2 are now in common use
    case M_SOF3:  return ("M_SOF3");
    case M_SOF5:  return ("M_SOF5");            // NB: codes C4 and CC are NOT SOF markers
    case M_SOF6:  return ("M_SOF6");
    case M_SOF7:  return ("M_SOF7");
    case M_SOF9:  return ("M_SOF9");
    case M_SOF10:  return ("M_SOF10");
    case M_SOF11:  return ("M_SOF11");
    case M_SOF13:  return ("M_SOF13");
    case M_SOF14:  return ("M_SOF14");
    case M_SOF15:  return ("M_SOF15");
    case M_SOI:  return ("M_SOI");               // Start Of Image (beginning of datastream)
    case M_EOI:  return ("M_EOI");               // End Of Image (end of datastream)
    case M_SOS:  return ("M_SOS");               // Start Of Scan (begins compressed data)
    case M_JFIF:  return ("M_JFIF");             // Jfif marker
    case M_EXIF:  return ("M_EXIF");             // Exif marker.  Also used for XMP data!
        //       case M_XMP:  message("M_XMP"); break;           // Not a real tag (same value in file as Exif!)
    case M_APP2:  return ("M_APP2");
    case M_APP3:  return ("M_APP3");             // Nochmal Exif???
    case M_APP4:  return ("M_APP4");
    case M_APP5:  return ("M_APP5");
    case M_APP6:  return ("M_APP6");
    case M_APP7:  return ("M_APP7");
    case M_APP8:  return ("M_APP8");
    case M_APP9:  return ("M_APP9");
    case M_APP10:  return ("M_APP10");
    case M_APP11:  return ("M_APP11");
    case M_APP12:  return ("M_APP12");
    case M_IPTC:  return ("M_IPTC");          // IPTC marker
    case M_APP14:  return ("M_APP14");        // Oft für Copyright-Einträge
    case M_APP15:  return ("M_APP15");
    case M_COM:  return ("M_COM");              // COMment
    case M_DQT:  return ("M_DQT");
    case M_DHT:  return ("M_DHT");
    case M_DRI:  return ("M_DRI");
    default:
        QString text = "00"+QString::number(id,16);
        return ("Unknown: 0x"+text.right(2));
    }

    return "ERROR";
}

// Detailed info about TAGS may be found at
// http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/JPEG.html
QString QMetaData::tagToText(QExifImageHeader::ImageTag tag){
    switch (tag) {
    case 0x0100: return tr("ImageWidth");
    case 0x0101: return tr("ImageLength");
    case 0x0102: return tr("BitsPerSample");
    case 0x0103: return tr("Compression");
    case 0x0106: return tr("PhotometricInterpretation");
    case 0x0112: return tr("Orientation");
    case 0x0115: return tr("SamplesPerPixel");
    case 0x011C: return tr("PlanarConfiguration");
    case 0x0212: return tr("YCbCrSubSampling");
    case 0x011A: return tr("XResolution");
    case 0x011B: return tr("YResolution");
    case 0x0128: return tr("ResolutionUnit");
    case 0x0111: return tr("StripOffsets");
    case 0x0116: return tr("RowsPerStrip");
    case 0x0117: return tr("StripByteCounts");
    case 0x012D: return tr("TransferFunction");
    case 0x013E: return tr("WhitePoint");
    case 0x013F: return tr("PrimaryChromaciticies");
    case 0x0211: return tr("YCbCrCoefficients");
    case 0x0213: return tr("YCbCrPositioning");
    case 0x0214: return tr("ReferenceBlackWhite");
    case 0x0132: return tr("DateTime");
    case 0x010E: return tr("ImageDescription");
    case 0x010F: return tr("Make");
    case 0x0110: return tr("Model");
    case 0x0131: return tr("Software");
    case 0x013B: return tr("Artist");
    case 0x8298: return tr("Copyright");
    default:
        QString text = "0000"+QString::number(tag,16);
        return ("Unknown tag: 0x"+text.right(4));
    }

    return "ERROR";
}
QString QMetaData::tagToText(QExifImageHeader::ExifExtendedTag tag){
    switch (tag) {
    case 0x9000: return tr("ExifVersion");
    case 0xA000: return tr("FlashPixVersion");
    case 0xA001: return tr("ColorSpace");
    case 0x9101: return tr("ComponentsConfiguration");
    case 0x9102: return tr("CompressedBitsPerPixel");
    case 0xA002: return tr("PixelXDimension");
    case 0xA003: return tr("PixelYDimension");
    case 0x927C: return tr("MakerNote");
    case 0x9286: return tr("UserComment");
    case 0xA004: return tr("RelatedSoundFile");
    case 0x9003: return tr("DateTimeOriginal");
    case 0x9004: return tr("DateTimeDigitized");
    case 0x9290: return tr("SubSecTime");
    case 0x9291: return tr("SubSecTimeOriginal");
    case 0x9292: return tr("SubSecTimeDigitized");
    case 0xA420: return tr("ImageUniqueId");
    case 0x829A: return tr("ExposureTime");
    case 0x829D: return tr("FNumber");
    case 0x8822: return tr("ExposureProgram");
    case 0x8824: return tr("SpectralSensitivity");
    case 0x8827: return tr("ISOSpeedRatings");
    case 0x8828: return tr("Oecf");
    case 0x9201: return tr("ShutterSpeedValue");
    case 0x9202: return tr("ApertureValue");
    case 0x9203: return tr("BrightnessValue");
    case 0x9204: return tr("ExposureBiasValue");
    case 0x9205: return tr("MaxApertureValue");
    case 0x9206: return tr("SubjectDistance");
    case 0x9207: return tr("MeteringMode");
    case 0x9208: return tr("LightSource");
    case 0x9209: return tr("Flash");
    case 0x920A: return tr("FocalLength");
    case 0x9214: return tr("SubjectArea");
    case 0xA20B: return tr("FlashEnergy");
    case 0xA20C: return tr("SpatialFrequencyResponse");
    case 0xA20E: return tr("FocalPlaneXResolution");
    case 0xA20F: return tr("FocalPlaneYResolution");
    case 0xA210: return tr("FocalPlaneResolutionUnit");
    case 0xA214: return tr("SubjectLocation");
    case 0xA215: return tr("ExposureIndex");
    case 0xA217: return tr("SensingMethod");
    case 0xA300: return tr("FileSource");
    case 0xA301: return tr("SceneType");
    case 0xA302: return tr("CfaPattern");
    case 0xA401: return tr("CustomRendered");
    case 0xA402: return tr("ExposureMode");
    case 0xA403: return tr("WhiteBalance");
    case 0xA404: return tr("DigitalZoomRatio");
    case 0xA405: return tr("FocalLengthIn35mmFilm");
    case 0xA406: return tr("SceneCaptureType");
    case 0xA407: return tr("GainControl");
    case 0xA408: return tr("Contrast");
    case 0xA409: return tr("Saturation");
    case 0xA40A: return tr("Sharpness");
    case 0xA40B: return tr("DeviceSettingDescription");
    case 0x40C: return tr("SubjectDistanceRange");
    default: return tr("Unknown extended tag");
    }

    return "ERROR";
}
QString QMetaData::tagToText(QExifImageHeader::GpsTag tag){
    switch (tag) {
    case 0x0000: return tr("GpsVersionId");
    case 0x0001: return tr("GpsLatitudeRef");
    case 0x0002: return tr("GpsLatitude");
    case 0x0003: return tr("GpsLongitudeRef");
    case 0x0004: return tr("GpsLongitude");
    case 0x0005: return tr("GpsAltitudeRef");
    case 0x0006: return tr("GpsAltitude");
    case 0x0007: return tr("GpsTimeStamp");
    case 0x0008: return tr("GpsSatellites");
    case 0x0009: return tr("GpsStatus");
    case 0x000A: return tr("GpsMeasureMode");
    case 0x000B: return tr("GpsDop");
    case 0x000C: return tr("GpsSpeedRef");
    case 0x000D: return tr("GpsSpeed");
    case 0x000E: return tr("GpsTrackRef");
    case 0x000F: return tr("GpsTrack");
    case 0x0010: return tr("GpsImageDirectionRef");
    case 0x0011: return tr("GpsImageDirection");
    case 0x0012: return tr("GpsMapDatum");
    case 0x0013: return tr("GpsDestLatitudeRef");
    case 0x0014: return tr("GpsDestLatitude");
    case 0x0015: return tr("GpsDestLongitudeRef");
    case 0x0016: return tr("GpsDestLongitude");
    case 0x0017: return tr("GpsDestBearingRef");
    case 0x0018: return tr("GpsDestBearing");
    case 0x0019: return tr("GpsDestDistanceRef");
    case 0x001A: return tr("GpsDestDistance");
    case 0x001B: return tr("GpsProcessingMethod");
    case 0x001C: return tr("GpsAreaInformation");
    case 0x001D: return tr("GpsDateStamp");
    case 0x001E: return tr("GpsDifferential");
    default: return tr("Unknown GPS tag");
    }

    return "ERROR";
}
