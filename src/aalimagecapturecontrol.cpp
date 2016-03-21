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
#include "aalviewfindersettingscontrol.h"
#include "storagemanager.h"

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>

#include <QDir>
#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QStandardPaths>
#include <QDateTime>
#include <QGuiApplication>
#include <QScreen>
#include <QSettings>
#include <QtConcurrent/QtConcurrent>

AalImageCaptureControl::AalImageCaptureControl(AalCameraService *service, QObject *parent)
   : QCameraImageCaptureControl(parent),
    m_service(service),
    m_cameraControl(service->cameraControl()),
    m_lastRequestId(0),
    m_ready(false),
    m_targetFileName(),
    m_captureCancelled(false),
    m_screenAspectRatio(0.0),
    m_audioPlayer(new QMediaPlayer(this))
{
    m_galleryPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    m_audioPlayer->setMedia(QUrl::fromLocalFile("/system/media/audio/ui/camera_click.ogg"));
    m_audioPlayer->setAudioRole(QAudio::NotificationRole);

    QObject::connect(&m_storageManager, &StorageManager::previewReady,
                     this, &AalImageCaptureControl::imageCaptured);
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

    m_targetFileName = fileName;
    m_captureCancelled = false;

    AalMetaDataWriterControl* metadataControl = m_service->metadataWriterControl();
    int rotation = metadataControl->correctedOrientation();
    android_camera_set_rotation(m_service->androidControl(), rotation);

    android_camera_take_snapshot(m_service->androidControl());

    m_service->updateCaptureReady();

    return m_lastRequestId;
}

void AalImageCaptureControl::cancelCapture()
{
    m_captureCancelled = true;
    m_targetFileName.clear();
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

    // Copy the data buffer so that it is safe to pass it off to another thread,
    // since it will be destroyed once this function returns
    QByteArray dataCopy((const char*)data, data_size);

    QMetaObject::invokeMethod(AalCameraService::instance()->imageCaptureControl(),
                              "saveJpeg", Qt::QueuedConnection,
                              Q_ARG(QByteArray, dataCopy));
}

void AalImageCaptureControl::init(CameraControl *control, CameraControlListener *listener)
{
    Q_UNUSED(control);

    listener->on_msg_shutter_cb = &AalImageCaptureControl::shutterCB;
    listener->on_data_compressed_image_cb = &AalImageCaptureControl::saveJpegCB;

    connect(m_service->videoOutputControl(), SIGNAL(previewReady()), this, SLOT(onPreviewReady()));
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
    return !m_targetFileName.isEmpty();
}

void AalImageCaptureControl::shutter()
{
    bool playShutterSound = m_settings.value("playShutterSound", true).toBool();
    if (playShutterSound) {
        m_audioPlayer->play();
    }
    Q_EMIT imageExposed(m_lastRequestId);
}

void AalImageCaptureControl::saveJpeg(const QByteArray& data)
{
    if (m_captureCancelled) {
        m_captureCancelled = false;
        return;
    }

    // Copy the metadata so that we can clear its container
    QVariantMap metadata;
    AalMetaDataWriterControl* metadataControl = m_service->metadataWriterControl();
    Q_FOREACH(QString key, metadataControl->availableMetaData()) {
        metadata.insert(key, metadataControl->metaData(key));
    }
    m_service->metadataWriterControl()->clearAllMetaData();

    QString fileName = m_targetFileName;
    m_targetFileName.clear();

    AalViewfinderSettingsControl* viewfinder = m_service->viewfinderControl();
    QSize resolution = viewfinder->viewfinderParameter(QCameraViewfinderSettingsControl::Resolution).toSize();

    // Restart the viewfinder and notify that the camera is ready to capture again
    if (m_service->androidControl()) {
        android_camera_start_preview(m_service->androidControl());
    }
    m_service->updateCaptureReady();

    DiskWriteWatcher* watcher = new DiskWriteWatcher(this);
    QObject::connect(watcher, &QFutureWatcher<QString>::finished, this, &AalImageCaptureControl::onImageFileSaved);
    m_pendingSaveOperations.insert(watcher, m_lastRequestId);

    QFuture<SaveToDiskResult> future = QtConcurrent::run(&m_storageManager, &StorageManager::saveJpegImage,
                                                         data, metadata, fileName, resolution, m_lastRequestId);
    watcher->setFuture(future);
}

void AalImageCaptureControl::onImageFileSaved()
{
    DiskWriteWatcher* watcher = static_cast<DiskWriteWatcher*>(sender());

    if (m_pendingSaveOperations.contains(watcher)) {
        int requestID = m_pendingSaveOperations.take(watcher);

        SaveToDiskResult result = watcher->result();
        delete watcher;

        if (result.success) {
            Q_EMIT imageSaved(requestID, result.fileName);
        } else {
            Q_EMIT error(requestID, QCameraImageCapture::ResourceError, result.errorMessage);
        }
    }
}
