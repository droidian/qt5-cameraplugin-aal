/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "storagemanager.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QBuffer>
#include <QImageReader>

#include <exiv2/exiv2.hpp>
#include <cmath>

const QLatin1String photoBase = QLatin1String("image");
const QLatin1String videoBase = QLatin1String("video");
const QLatin1String photoExtension = QLatin1String("jpg");
const QLatin1String videoExtension = QLatin1String("mp4");
const QLatin1String dateFormat = QLatin1String("yyyyMMdd_HHmmsszzz");

StorageManager::StorageManager(QObject* parent) : QObject(parent)
{
}

QString StorageManager::nextPhotoFileName(const QString &directoy)
{
    m_directory = directoy;
    if (m_directory.isEmpty()) {
        m_directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/" + QCoreApplication::applicationName();
        QDir dir;
        dir.mkpath(m_directory);
    }

    return fileNameGenerator(photoBase, photoExtension);
}

QString StorageManager::nextVideoFileName(const QString &directoy)
{
    m_directory = directoy;
    if (m_directory.isEmpty()) {
        m_directory = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/" + QCoreApplication::applicationName();
        QDir dir;
        dir.mkpath(m_directory);
    }

    return fileNameGenerator(videoBase, videoExtension);
}

bool StorageManager::checkDirectory(const QString &path) const
{
    QFileInfo fi(path);
    QDir dir;
    if (fi.isDir())
        dir.setPath(path);
    else
        dir.setPath(fi.absoluteDir().absolutePath());

    if (!dir.exists()) {
        bool ok = dir.mkpath(dir.absolutePath());
        if (!ok)
            return false;
    }

    fi.setFile(dir.absolutePath());
    if (!fi.isWritable())
        return false;

    return true;
}

QString StorageManager::fileNameGenerator(const QString &base, const QString& extension)
{
    QString date = QDateTime::currentDateTime().toString(dateFormat);
    return QString("%1/%2%3.%4")
            .arg(m_directory)
            .arg(base)
            .arg(date)
            .arg(extension);
}

bool StorageManager::updateJpegMetadata(QByteArray data, QVariantMap metadata, QTemporaryFile* destination)
{
    if (data.isEmpty() || destination == 0) return false;

    Exiv2::Image::AutoPtr image;
    try {
        image = Exiv2::ImageFactory::open(static_cast<const Exiv2::byte*>((const unsigned char*)data.constData()), data.size());
        if (!image.get()) {
            return false;
        }
    } catch(const Exiv2::AnyError&) {
        return false;
    }

    try {
        image->readMetadata();
        Exiv2::ExifData ed = image->exifData();

        /* This works around the Exiv2's unability to deal with the MakerNote tag
         * due to its totally unspecified nature.
         * Its presence sometimes causes Exiv2 to refuse to write/modify any other
         * metadata (including orientation, GPS metadata).
         * See https://bugs.launchpad.net/zhongshan/+bug/1572878 for possible
         * consequences.
         */
        Exiv2::ExifKey makerNoteKey = Exiv2::ExifKey("Exif.Photo.MakerNote");
        Exiv2::ExifData::iterator makerNoteIter = ed.findKey(makerNoteKey);
        if (makerNoteIter != ed.end()) {
            ed.erase(makerNoteIter);
        }

        const QString now = QDateTime::currentDateTime().toString("yyyy:MM:dd HH:mm:ss");
        ed["Exif.Photo.DateTimeOriginal"].setValue(now.toStdString());
        ed["Exif.Photo.DateTimeDigitized"].setValue(now.toStdString());

        if (metadata.contains("GPSLatitude") &&
            metadata.contains("GPSLongitude") &&
            metadata.contains("GPSTimeStamp")) {

            // Write all GPS metadata according to version 2.2 of the EXIF spec,
            // which is what Android did. See: http://www.exiv2.org/Exif2-2.PDF
            const char version[4] = {2, 2, 0, 0};
            Exiv2::DataValue versionValue(Exiv2::unsignedByte);
            versionValue.read((const Exiv2::byte*)version, 4);
            ed.add(Exiv2::ExifKey("Exif.GPSInfo.GPSVersionID"), &versionValue);

            // According to the spec, the GPS processing method is a buffer of type Undefined, which
            // does not need to be zero terminated. It should be prepended by an 8 byte, zero padded
            // string specifying the encoding.
            const char methodHeader[8] = {'A', 'S', 'C', 'I', 'I', 0, 0, 0};
            QByteArray method = metadata.value("GPSProcessingMethod").toString().toLatin1();
            method.prepend(methodHeader, 8);
            Exiv2::DataValue methodValue(Exiv2::undefined);
            methodValue.read((const Exiv2::byte*)method.constData(), method.size());
            ed.add(Exiv2::ExifKey("Exif.GPSInfo.GPSProcessingMethod"), &methodValue);

            double latitude = metadata.value("GPSLatitude").toDouble();
            ed["Exif.GPSInfo.GPSLatitude"] = decimalToExifRational(latitude).toStdString();
            ed["Exif.GPSInfo.GPSLatitudeRef"] = (latitude < 0 ) ? "S" : "N";

            double longitude = metadata.value("GPSLongitude").toDouble();
            ed["Exif.GPSInfo.GPSLongitude"] = decimalToExifRational(longitude).toStdString();
            ed["Exif.GPSInfo.GPSLongitudeRef"] = (longitude < 0 ) ? "W" : "E";

            if (metadata.contains("GPSAltitude")) {
                // Assume altitude precision to the meter
                unsigned int altitude = floor(metadata.value("GPSAltitude").toDouble());
                Exiv2::URationalValue::AutoPtr altitudeValue(new Exiv2::URationalValue);
                altitudeValue->value_.push_back(std::make_pair(altitude,1));
                ed.add(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitude"), altitudeValue.get());

                // Byte field of lenght 1. Value of 0 means the reference is sea level.
                const char reference = 0;
                Exiv2::DataValue referenceValue(Exiv2::unsignedByte);
                referenceValue.read((const Exiv2::byte*) &reference, 1);
                ed.add(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitudeRef"), &referenceValue);
            }

            QDateTime stamp = metadata.value("GPSTimeStamp").toDateTime();
            ed["Exif.GPSInfo.GPSTimeStamp"] = stamp.toString("HH/1 mm/1 ss/1").toStdString();
            ed["Exif.GPSInfo.GPSDateStamp"] = stamp.toString("yyyy:MM:dd").toStdString();
        }

        image->setExifData(ed);
        image->writeMetadata();
    } catch(const Exiv2::AnyError&) {
        return false;
    }

    if (!destination->open()) {
        return false;
    }

    try {
        Exiv2::BasicIo& io = image->io();
        char* modifiedMetadata = reinterpret_cast<char*>(io.mmap());
        const long size = io.size();
        const qint64 writtenSize = destination->write(modifiedMetadata, size);
        io.munmap();
        destination->close();
        return (writtenSize == size);

    } catch(const Exiv2::AnyError&) {
        destination->close();
        return false;
    }
}

SaveToDiskResult StorageManager::saveJpegImage(QByteArray data, QVariantMap metadata, QString fileName,
                                               QSize previewResolution, int captureID)
{
    SaveToDiskResult result;

    QString captureFile;
    QFileInfo fi(fileName);
    if (fileName.isEmpty() || fi.isDir()) {
        captureFile = nextPhotoFileName(fileName);
    } else {
        captureFile = fileName;
    }
    result.fileName = captureFile;

    bool diskOk = checkDirectory(captureFile);
    if (!diskOk) {
        result.errorMessage = QString("Won't be able to save file %1 to disk").arg(captureFile);
        return result;
    }

    QBuffer buffer(&data);
    QImageReader reader(&buffer, "jpg");

    QSize scaledSize = reader.size(); // fast, as it does not decode the JPEG
    scaledSize.scale(previewResolution, Qt::KeepAspectRatio);
    reader.setScaledSize(scaledSize);
    reader.setQuality(25);
    QImage image = reader.read();
    Q_EMIT previewReady(captureID, image);

    QTemporaryFile file;
    if (!updateJpegMetadata(data, metadata, &file)) {
        qWarning() << "Failed to update EXIF timestamps. Picture will be saved as UTC timezone.";
        if (!file.open()) {
            result.errorMessage = QString("Could not open temprary file %1").arg(file.fileName());
            return result;
        }

        const qint64 writtenSize = file.write(data);
        file.close();
        if (writtenSize != data.size()) {
            result.errorMessage = QString("Could not write file %1").arg(fileName);
            return result;
        }
    }

    QFile finalFile(file.fileName());
    bool ok = finalFile.rename(captureFile);
    if (!ok) {
        result.errorMessage = QString("Could not save image to %1").arg(fileName);
        return result;
    }

    result.success = true;
    return result;
}

QString StorageManager::decimalToExifRational(double decimal)
{
    decimal = fabs(decimal);
    unsigned int degrees = floor(decimal);
    unsigned int minutes = floor((decimal - degrees) * 60);
    double seconds = (decimal - degrees - minutes / 60) * 3600;
    seconds = floor(seconds * 100);

    return QString("%1/1 %2/1 %3/100").arg(degrees).arg(minutes).arg(seconds);
}

SaveToDiskResult::SaveToDiskResult() : success(false)
{
}
