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

#include "aalcameraservice.h"
#include "aalimagecapturecontrol.h"
#include "aalimageencodercontrol.h"
#include "aalmetadatawritercontrol.h"
#include "aalvideorenderercontrol.h"
#include "storagemanager.h"

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QDateTime>

#include <cmath>

#include <ubuntu/application/ui/display.h>

AalImageCaptureControl::AalImageCaptureControl(AalCameraService *service, QObject *parent)
   : QCameraImageCaptureControl(parent),
    m_service(service),
    m_cameraControl(service->cameraControl()),
    m_lastRequestId(0),
    m_ready(false),
    m_pendingCaptureFile(),
    m_photoWidth(320),
    m_photoHeight(240),
    m_aspectRatio(0.0),
    m_screenAspectRatio(0.0),
    m_audioPlayer(new QMediaPlayer(this))
{
    m_galleryPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    m_audioPlayer->setMedia(QUrl::fromLocalFile("/system/media/audio/ui/camera_click.ogg"));
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
        availableMetadata.contains("GPSAltitude") &&
        availableMetadata.contains("GPSTimeStamp")) {
        float latitude = metadataControl->metaData("GPSLatitude").toFloat();
        float longitude = metadataControl->metaData("GPSLongitude").toFloat();
        float altitude = metadataControl->metaData("GPSAltitude").toFloat();
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

    // Set the optimal image resolution that will be used by the camera
    QImageEncoderSettings settings;
    AalImageEncoderControl *imageEncoderControl = AalCameraService::instance()->imageEncoderControl();
    if (!imageEncoderControl->supportedResolutions(settings).empty()) {
        imageEncoderControl->setSize(
                chooseOptimalSize(imageEncoderControl->supportedResolutions(settings)));
    }
    else
        qWarning() << "No supported resolutions detected for currently selected camera device." << endl;

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

float AalImageCaptureControl::getAspectRatio() const
{
    return m_aspectRatio;
}

void AalImageCaptureControl::shutter()
{
    m_audioPlayer->play();
    Q_EMIT imageExposed(m_lastRequestId);
}

QSize AalImageCaptureControl::chooseOptimalSize(const QList<QSize> &sizes)
{
    QSize optimalSize;
    long optimalPixels = 0;

    if (!sizes.empty()) {
        getPriorityAspectRatios();
        m_aspectRatio = m_prioritizedAspectRatios.front();

        // Loop over all reported camera resolutions until we find the highest
        // one that matches the current prioritized aspect ratio. If it doesn't
        // find one on the current aspect ration, it selects the next ratio and
        // tries again.
        QList<float>::const_iterator ratioIt = m_prioritizedAspectRatios.begin();
        while (ratioIt != m_prioritizedAspectRatios.end()) {
            m_aspectRatio = (*ratioIt);

            QList<QSize>::const_iterator it = sizes.begin();
            while (it != sizes.end()) {
                const float ratio = (float)(*it).width() / (float)(*it).height();
                const long pixels = ((long)((*it).width())) * ((long)((*it).height()));
                const float EPSILON = 10e-3;
                if (fabs(ratio - m_aspectRatio) < EPSILON && pixels > optimalPixels) {
                    optimalSize = *it;
                    optimalPixels = pixels;
                }
                ++it;
            }
            if (optimalPixels > 0) break;
            ++ratioIt;
        }
    }

    return optimalSize;
}

float AalImageCaptureControl::getScreenAspectRatio()
{
    // Only get the screen aspect ratio once, otherwise use the cached copy
    if (m_screenAspectRatio == 0.0) {
        // Get screen resolution.
        UAUiDisplay* display = ua_ui_display_new_with_index(0);
        const int kScreenWidth = ua_ui_display_query_horizontal_res(display);
        const int kScreenHeight = ua_ui_display_query_vertical_res(display);
        Q_ASSERT(kScreenWidth > 0 && kScreenHeight > 0);

        ua_ui_display_destroy(display);

        m_screenAspectRatio = (kScreenWidth > kScreenHeight) ?
            ((float)kScreenWidth / (float)kScreenHeight) : ((float)kScreenHeight / (float)kScreenWidth);
    }

    return m_screenAspectRatio;
}

void AalImageCaptureControl::getPriorityAspectRatios()
{
    m_prioritizedAspectRatios.clear();

    if (m_service->isBackCameraUsed()) {
        if (m_screenAspectRatio > 0.0f) {
            m_prioritizedAspectRatios.append(getScreenAspectRatio());
        }
        // Prioritized list of aspect ratios for the back camera
        const float backAspectRatios[4] = { 16.0f/9.0f, 3.0f/2.0f, 4.0f/3.0f, 5.0f/4.0f };
        for (uint8_t i=0; i<4; ++i) {
            if (!m_prioritizedAspectRatios.contains(backAspectRatios[i])) {
                m_prioritizedAspectRatios.append(backAspectRatios[i]);
            }
        }
    } else {
        // Prioritized list of aspect ratios for the front camera
        const float frontAspectRatios[4] = { 4.0f/3.0f, 5.0f/4.0f, 16.0f/9.0f, 3.0f/2.0f };
        for (uint8_t i=0; i<4; ++i) {
            m_prioritizedAspectRatios.append(frontAspectRatios[i]);
        }
    }
}

void AalImageCaptureControl::saveJpeg(void *data, uint32_t dataSize)
{
    if (m_pendingCaptureFile.isNull() || !m_service->androidControl())
        return;

    QTemporaryFile file;
    if (!file.open()) {
        emit error(m_lastRequestId, QCameraImageCapture::ResourceError,
                   QString("Could not open temprary file %1").arg(file.fileName()));
        m_pendingCaptureFile.clear();
        m_service->updateCaptureReady();
        return;
    }

    qint64 writtenSize = file.write((const char*)data, dataSize);
    file.close();
    if (writtenSize != dataSize) {
        emit error(m_lastRequestId, QCameraImageCapture::ResourceError,
                   QString("Could not write file %1").arg(file.fileName()));
        m_pendingCaptureFile.clear();
        m_service->updateCaptureReady();
        return;
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
