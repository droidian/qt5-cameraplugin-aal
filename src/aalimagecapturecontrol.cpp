/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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

#include "aalcameraservice.h"
#include "aalimagecapturecontrol.h"
#include "aalimageencodercontrol.h"
#include "aalmetadatawritercontrol.h"
#include "aalvideorenderercontrol.h"
#include "storagemanager.h"

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>
#include <exiv2/exiv2.hpp>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QStandardPaths>
#include <QDateTime>
#include <QGuiApplication>
#include <QScreen>
#include <QSettings>

#include <cmath>

AalImageCaptureControl::AalImageCaptureControl(AalCameraService *service, QObject *parent)
   : QCameraImageCaptureControl(parent),
    m_service(service),
    m_cameraControl(service->cameraControl()),
    m_lastRequestId(0),
    m_ready(false),
    m_pendingCaptureFile(),
    m_captureCancelled(false),
    m_screenAspectRatio(0.0),
    m_audioPlayer(new QMediaPlayer(this))
{
    m_galleryPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    m_audioPlayer->setMedia(QUrl::fromLocalFile("/system/media/audio/ui/camera_click.ogg"));
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
    m_audioPlayer->setAudioRole(QMediaPlayer::AlertRole);
#else
    m_audioPlayer->setAudioRole(QAudio::NotificationRole);
#endif
}

AalImageCaptureControl::~AalImageCaptureControl()
{
    delete(m_audioPlayer);
}

bool AalImageCaptureControl::isReadyForCapture() const
{
    return m_ready;
}

int AalImageCaptureControl::capture(const QString &fileName)
{
    m_lastRequestId++;
    if (!m_ready || !m_service->androidControl()) {
        emit error(m_lastRequestId, QCameraImageCapture::NotReadyError,
                   QLatin1String("Camera not ready to capture"));
        return m_lastRequestId;
    }

    m_captureCancelled = false;
    QFileInfo fi(fileName);
    if (fileName.isEmpty() || fi.isDir()) {
        m_pendingCaptureFile = m_storageManager.nextPhotoFileName(fileName);
    } else {
        m_pendingCaptureFile = fileName;
    }
    bool diskOk = m_storageManager.checkDirectory(m_pendingCaptureFile);
    if (!diskOk) {
        emit error(m_lastRequestId, QCameraImageCapture::ResourceError,
                   QString("Won't be able to save file %1 to disk").arg(m_pendingCaptureFile));
        return m_lastRequestId;
    }

    android_camera_take_snapshot(m_service->androidControl());

    m_service->updateCaptureReady();

    m_service->videoOutputControl()->createPreview();

    return m_lastRequestId;
}

void AalImageCaptureControl::cancelCapture()
{
    m_captureCancelled = true;
    m_pendingCaptureFile.clear();
}

void AalImageCaptureControl::shutterCB(void *context)
{
    Q_UNUSED(context);
    QMetaObject::invokeMethod(AalCameraService::instance()->imageCaptureControl(),
                              "shutter", Qt::QueuedConnection);
}

void AalImageCaptureControl::saveJpegCB(void *data, uint32_t data_size, void *context)
{
    Q_UNUSED(context);
    AalCameraService::instance()->imageCaptureControl()->saveJpeg(data, data_size);
}

void AalImageCaptureControl::init(CameraControl *control, CameraControlListener *listener)
{
    Q_UNUSED(control);

    listener->on_msg_shutter_cb = &AalImageCaptureControl::shutterCB;
    listener->on_data_compressed_image_cb = &AalImageCaptureControl::saveJpegCB;

    connect(m_service->videoOutputControl(), SIGNAL(previewReady()), this, SLOT(onPreviewReady()));
}

void AalImageCaptureControl::onPreviewReady()
{
    // The preview image was fully captured, notify the UI layer
    Q_EMIT imageCaptured(m_lastRequestId, m_service->videoOutputControl()->preview());
}

void AalImageCaptureControl::setReady(bool ready)
{
    if (m_ready != ready) {
        m_ready = ready;
        Q_EMIT readyForCaptureChanged(m_ready);
    }
}

bool AalImageCaptureControl::isCaptureRunning() const
{
    return !m_pendingCaptureFile.isNull();
}

void AalImageCaptureControl::shutter()
{
    bool playShutterSound = m_settings.value("playShutterSound", true).toBool();
    if (playShutterSound) {
        m_audioPlayer->play();
    }
    Q_EMIT imageExposed(m_lastRequestId);
}

bool AalImageCaptureControl::updateJpegMetadata(void* data, uint32_t dataSize, QTemporaryFile* destination)
{
    if (data == 0 || destination == 0) return false;

    Exiv2::Image::AutoPtr image;
    try {
        image = Exiv2::ImageFactory::open(static_cast<Exiv2::byte*>(data), dataSize);
        if (!image.get()) {
            return false;
        }
    } catch(const Exiv2::AnyError&) {
        return false;
    }

    AalMetaDataWriterControl* metadataControl = m_service->metadataWriterControl();

    try {
        image->readMetadata();
        Exiv2::ExifData ed = image->exifData();
        const QString now = QDateTime::currentDateTime().toString("yyyy:MM:dd HH:mm:ss");
        ed["Exif.Photo.DateTimeOriginal"].setValue(now.toStdString());
        ed["Exif.Photo.DateTimeDigitized"].setValue(now.toStdString());

        AalMetaDataWriterControl* metadataControl = m_service->metadataWriterControl();
        int rotation = metadataControl->correctedOrientation();
        int orientation = rotationToExifOrientation(rotation);

        // FIXME: this is not the correct orientation. there seem to be a 90 degree
        // difference that was not there when android was writing it.
        ed["Exif.Image.Orientation"] = uint16_t(orientation);

        // FIXME: the latitude and longitude are for some reason rounded down to 5 decimals
        // precision, while in QML we set them without this rounding. Figure out where the loss
        // or precision happens.
        QStringList availableMetadata = metadataControl->availableMetaData();
        if (availableMetadata.contains("GPSLatitude") &&
            availableMetadata.contains("GPSLongitude") &&
            availableMetadata.contains("GPSTimeStamp")) {

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
            QByteArray method = metadataControl->metaData("GPSProcessingMethod").toString().toLatin1();
            method.prepend(methodHeader, 8);
            Exiv2::DataValue methodValue(Exiv2::undefined);
            methodValue.read((const Exiv2::byte*)method.constData(), method.size());
            ed.add(Exiv2::ExifKey("Exif.GPSInfo.GPSProcessingMethod"), &methodValue);

            double latitude = metadataControl->metaData("GPSLatitude").toDouble();
            ed["Exif.GPSInfo.GPSLatitude"] = decimalToExifRational(latitude).toStdString();
            ed["Exif.GPSInfo.GPSLatitudeRef"] = (latitude < 0 ) ? "S" : "N";

            double longitude = metadataControl->metaData("GPSLongitude").toDouble();
            ed["Exif.GPSInfo.GPSLongitude"] = decimalToExifRational(longitude).toStdString();
            ed["Exif.GPSInfo.GPSLongitudeRef"] = (longitude < 0 ) ? "W" : "E";

            if (availableMetadata.contains("GPSAltitude")) {
                // Assume altitude precision to the meter
                unsigned int altitude = floor(metadataControl->metaData("GPSAltitude").toDouble());
                Exiv2::URationalValue::AutoPtr altitudeValue(new Exiv2::URationalValue);
                altitudeValue->value_.push_back(std::make_pair(altitude,1));
                ed.add(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitude"), altitudeValue.get());

                // Byte field of lenght 1. Value of 0 means the reference is sea level.
                const char reference = 0;
                Exiv2::DataValue referenceValue(Exiv2::unsignedByte);
                referenceValue.read((const Exiv2::byte*) &reference, 1);
                ed.add(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitudeRef"), &referenceValue);
            }

            QDateTime stamp = metadataControl->metaData("GPSTimeStamp").toDateTime();
            ed["Exif.GPSInfo.GPSTimeStamp"] = stamp.toString("HH/1 mm/1 ss/1").toStdString();
            ed["Exif.GPSInfo.GPSDateStamp"] = stamp.toString("yyyy:MM:dd").toStdString();   
        }

        image->setExifData(ed);
        image->writeMetadata();
    } catch(const Exiv2::AnyError&) {
        return false;
    }

    // FIXME: this is ok now, but will be too late when this code goes to a thread.
    // We need to copy this metadata and keep it around.
    m_service->metadataWriterControl()->clearAllMetaData();

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

int AalImageCaptureControl::rotationToExifOrientation(int rotation)
{
    if (rotation == 0) return 0;
    else if (rotation == 180) return 2;
    else if (rotation == 90) return 5;
    else if (rotation == -90 || rotation == 270) return 7;
    else {
        qWarning() << "Can not convert rotation of" << rotation << "degrees to EXIF orientation";
        return 0;
    }
}

QString AalImageCaptureControl::decimalToExifRational(double decimal)
{
    decimal = fabs(decimal);
    unsigned int degrees = floor(decimal);
    unsigned int minutes = floor((decimal - degrees) * 60);
    double seconds = (decimal - degrees - minutes / 60) * 3600;
    seconds = floor(seconds * 100);

    return QString("%1/1 %2/1 %3/100").arg(degrees).arg(minutes).arg(seconds);
}

void AalImageCaptureControl::saveJpeg(void *data, uint32_t dataSize)
{
    if (m_captureCancelled) {
        m_captureCancelled = false;
        return;
    }

    if (m_pendingCaptureFile.isNull() || !m_service->androidControl())
        return;

    QTemporaryFile file;
    if (!updateJpegMetadata(data, dataSize, &file)) {
        qWarning() << "Failed to update EXIF timestamps. Picture will be saved as UTC timezone.";
        if (!file.open()) {
            emit error(m_lastRequestId, QCameraImageCapture::ResourceError,
                       QString("Could not open temprary file %1").arg(file.fileName()));
            m_pendingCaptureFile.clear();
            m_service->updateCaptureReady();
            return;
        }

        const qint64 writtenSize = file.write(static_cast<const char*>(data), dataSize);
        file.close();
        if (writtenSize != dataSize) {
            emit error(m_lastRequestId, QCameraImageCapture::ResourceError,
                       QString("Could not write file %1").arg(file.fileName()));
            m_pendingCaptureFile.clear();
            m_service->updateCaptureReady();
            return;
        }
    }

    QFile finalFile(file.fileName());
    bool ok = finalFile.rename(m_pendingCaptureFile);
    if (!ok) {
        emit error(m_lastRequestId, QCameraImageCapture::ResourceError,
                   QString("Could not save image to %1").arg(m_pendingCaptureFile));
        m_pendingCaptureFile.clear();
        m_service->updateCaptureReady();
        return;
    }

    Q_EMIT imageSaved(m_lastRequestId, m_pendingCaptureFile);
    m_pendingCaptureFile.clear();

    android_camera_start_preview(m_service->androidControl());
    m_service->updateCaptureReady();
}
