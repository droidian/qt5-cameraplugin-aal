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

    AalMetaDataWriterControl* metadataControl = m_service->metadataWriterControl();

    int rotation = metadataControl->correctedOrientation();
    android_camera_set_rotation(m_service->androidControl(), rotation);

    QStringList availableMetadata = metadataControl->availableMetaData();
    if (availableMetadata.contains("GPSLatitude") &&
        availableMetadata.contains("GPSLongitude") &&
        availableMetadata.contains("GPSTimeStamp")) {
        float latitude = metadataControl->metaData("GPSLatitude").toFloat();
        float longitude = metadataControl->metaData("GPSLongitude").toFloat();
        float altitude = 0.0f;
        if (availableMetadata.contains("GPSAltitude")) {
            altitude = metadataControl->metaData("GPSAltitude").toFloat();
        }
        QDateTime timestamp = metadataControl->metaData("GPSTimeStamp").toDateTime();
        QString processingMethod = metadataControl->metaData("GPSProcessingMethod").toString();
        android_camera_set_location(m_service->androidControl(),
                                    &latitude, &longitude, &altitude,
                                    timestamp.toTime_t(),
                                    processingMethod.toLocal8Bit().constData());
    }

    android_camera_take_snapshot(m_service->androidControl());

    m_service->updateCaptureReady();

    m_service->videoOutputControl()->createPreview();

    m_service->metadataWriterControl()->clearAllMetaData();

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

    try {
        image->readMetadata();
        Exiv2::ExifData ed = image->exifData();
        const QString now = QDateTime::currentDateTime().toString("yyyy:MM:dd HH:mm:ss");
        ed["Exif.Photo.DateTimeOriginal"].setValue(now.toStdString());
        ed["Exif.Photo.DateTimeDigitized"].setValue(now.toStdString());
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
