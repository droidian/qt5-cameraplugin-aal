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

#include "aalimageencodercontrol.h"
#include "aalcameracontrol.h"
#include "aalviewfindersettingscontrol.h"
#include "aalvideoencodersettingscontrol.h"
#include "aalimagecapturecontrol.h"
#include "aalcameraservice.h"

#include <hybris/camera/camera_compatibility_layer_capabilities.h>

#include <unistd.h>

#include <QCamera>
#include <QDebug>

AalImageEncoderControl::AalImageEncoderControl(AalCameraService *service, QObject *parent)
    : QImageEncoderControl(parent),
      m_service(service),
      m_currentSize(),
      m_currentThumbnailSize()
{
}

AalImageEncoderControl::~AalImageEncoderControl()
{
}

QString AalImageEncoderControl::imageCodecDescription(const QString &codec) const
{
    Q_UNUSED(codec);
    return QString();
}

QImageEncoderSettings AalImageEncoderControl::imageSettings() const
{
    return m_encoderSettings;
}

void AalImageEncoderControl::setImageSettings(const QImageEncoderSettings &settings)
{
    if (!settings.isNull()) {
        // JPEG quality
        m_encoderSettings.setQuality(settings.quality());
        if (m_service->androidControl()) {
            int jpegQuality = qtEncodingQualityToJpegQuality(settings.quality());
            android_camera_set_jpeg_quality(m_service->androidControl(), jpegQuality);
        }

        // codec
        if (!settings.codec().isNull()) {
            m_encoderSettings.setCodec(settings.codec());
        }

        // resolution
        if (!settings.resolution().isNull()) {
            setSize(settings.resolution());
        }

        // encoding options
        if (!settings.encodingOptions().isEmpty()) {
            m_encoderSettings.setEncodingOptions(settings.encodingOptions());
        }
    }
}

QStringList AalImageEncoderControl::supportedImageCodecs() const
{
    return QStringList();
}

QList<QSize> AalImageEncoderControl::supportedResolutions(const QImageEncoderSettings &settings, bool *continuous) const
{
    Q_UNUSED(continuous);
    Q_UNUSED(settings);

    return m_availableSizes;
}

QList<QSize> AalImageEncoderControl::supportedThumbnailResolutions(const QImageEncoderSettings &settings, bool *continuous) const
{
    Q_UNUSED(continuous);
    Q_UNUSED(settings);

    return m_availableThumbnailSizes;
}

float AalImageEncoderControl::getAspectRatio() const
{
    return (float)m_currentSize.width() / (float)m_currentSize.height();
}

void AalImageEncoderControl::init(CameraControl *control)
{
    Q_ASSERT(control != NULL);

    if (m_availableSizes.isEmpty()) {
        android_camera_enumerate_supported_picture_sizes(control, &AalImageEncoderControl::getPictureSizeCb, this);
        android_camera_enumerate_supported_thumbnail_sizes(control, &AalImageEncoderControl::getThumbnailSizeCb, this);
    }

    int jpegQuality;
    android_camera_get_jpeg_quality(control, &jpegQuality);
    m_encoderSettings.setQuality(jpegQualityToQtEncodingQuality(jpegQuality));

    if (m_availableSizes.empty()) {
        qWarning() << "(AalImageEncoderControl::init) No supported resolutions detected for currently selected camera device." << endl;
        return;
    }

    if (!m_currentSize.isValid() || !m_availableSizes.contains(m_currentSize)) {
        QSize greatestSize;
        foreach (const QSize &size, m_availableSizes) {
            if (size.width() * size.height() > greatestSize.width() * greatestSize.height()) {
                greatestSize = size;
            }
        }

        setSize(greatestSize);
    } else {
        setSize(m_currentSize);
    }
}

bool AalImageEncoderControl::setSize(const QSize &size)
{
    CameraControl *cc = m_service->androidControl();
    if (!cc) {
        m_currentSize = size;
        m_encoderSettings.setResolution(m_currentSize);
        return true;
    }

    if (!m_availableSizes.contains(size)) {
        qWarning() << "(AalImageEncoderControl::setSize) Size " << size << "is not supported by the camera";
        qWarning() << "(AalImageEncoderControl::setSize) Supported sizes are: " << m_availableSizes;
        return false;
    }

    m_currentSize = size;
    m_encoderSettings.setResolution(m_currentSize);
    if (m_service->cameraControl()->captureMode() == QCamera::CaptureStillImage) {
        m_service->viewfinderControl()->setAspectRatio(getAspectRatio());
    }

    // Select m_currentThumbnailSize so that its aspect ratio is the same
    // as m_currentSize's aspect ratio
    float imageAspectRatio = getAspectRatio();
    float thumbnailAspectRatio;

    // Set the optimal thumbnail image resolution that will be saved to the JPEG file
    if (!m_availableThumbnailSizes.empty()) {
        // Because EXIF thumbnails must be at most 64KB by specification, make sure that
        // we request thumbnails that are no bigger than 128x128x4 bytes.
        // Fixes bug https://bugs.launchpad.net/ubuntu/+source/camera-app/+bug/1519766
        if (imageAspectRatio >= 1.0) {
            m_currentThumbnailSize = QSize(128, (int)(128.0f / imageAspectRatio));
        } else {
            m_currentThumbnailSize = QSize((int)(128.0f * imageAspectRatio), 128);
        }
        thumbnailAspectRatio = (float)m_currentThumbnailSize.width() / (float)m_currentThumbnailSize.height();
    }

    // Thumbnails will appear squashed or stretched if not the same aspect ratio as the original image.
    // This will most likely be an incorrect size list supplied to qtubuntu-camera from the camera driver.
    if (imageAspectRatio != thumbnailAspectRatio) {
        qWarning() << "(AalImageEncoderControl::setSize) ** Image and thumbnail aspect ratios are different. Thumbnails will look wrong!";
    }

    android_camera_set_picture_size(cc, m_currentSize.width(), m_currentSize.height());
    android_camera_set_thumbnail_size(cc, m_currentThumbnailSize.width(), m_currentThumbnailSize.height());
    return true;
}

void AalImageEncoderControl::resetAllSettings()
{
    m_availableSizes.clear();
    m_availableThumbnailSizes.clear();
    m_currentSize = QSize();
    m_currentThumbnailSize = QSize();
}

/*!
 * \brief AalImageEncoderControl::enablePhotoMode prepares the camera to take photos
 */
void AalImageEncoderControl::enablePhotoMode()
{
    CameraControl *cc = m_service->androidControl();
    if (!cc || !m_currentSize.isValid()) {
        return;
    }
    android_camera_set_picture_size(cc, m_currentSize.width(), m_currentSize.height());
    android_camera_set_thumbnail_size(cc, m_currentThumbnailSize.width(), m_currentThumbnailSize.height());
}

void AalImageEncoderControl::getPictureSizeCb(void *ctx, int width, int height)
{
    if (ctx != NULL)
    {
        AalImageEncoderControl *self = static_cast<AalImageEncoderControl *>(ctx);
        self->getPictureSize(width, height);
    }
    else
        qWarning() << "ctx is NULL, cannot get supported camera resolutions." << endl;
}

void AalImageEncoderControl::getThumbnailSizeCb(void *ctx, int width, int height)
{
    if (ctx != NULL)
    {
        AalImageEncoderControl *self = static_cast<AalImageEncoderControl *>(ctx);
        self->getThumbnailSize(width, height);
    }
    else
        qWarning() << "ctx is NULL, cannot get supported thumbnail resolutions." << endl;
}

void AalImageEncoderControl::getPictureSize(int width, int height)
{
    m_availableSizes.append(QSize(width, height));
}

void AalImageEncoderControl::getThumbnailSize(int width, int height)
{
    m_availableThumbnailSizes.append(QSize(width, height));
}


QMultimedia::EncodingQuality AalImageEncoderControl::jpegQualityToQtEncodingQuality(int jpegQuality)
{
    QMultimedia::EncodingQuality quality;
    if (jpegQuality <= 40) {
        quality = QMultimedia::VeryLowQuality;
    } else if (jpegQuality <= 60) {
        quality = QMultimedia::LowQuality;
    } else if (jpegQuality <= 80) {
        quality = QMultimedia::NormalQuality;
    } else if (jpegQuality <= 90) {
        quality = QMultimedia::HighQuality;
    } else {
        quality = QMultimedia::VeryHighQuality;
    }
    return quality;
}

int AalImageEncoderControl::qtEncodingQualityToJpegQuality(QMultimedia::EncodingQuality quality)
{
    int jpegQuality = 100;
    switch (quality) {
    case QMultimedia::VeryLowQuality:
        jpegQuality = 40;
        break;
    case QMultimedia::LowQuality:
        jpegQuality = 60;
        break;
    case QMultimedia::NormalQuality:
        jpegQuality = 80;
        break;
    case QMultimedia::HighQuality:
        jpegQuality = 90;
        break;
    case QMultimedia::VeryHighQuality:
        jpegQuality = 100;
        break;
    }
    return jpegQuality;
}
