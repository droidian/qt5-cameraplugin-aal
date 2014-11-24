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

#include <QImageEncoderControl>
#include <QList>
#include <QString>
#include <QStringList>

#include <QDebug>

#include "aalimageencodercontrol.h"

AalImageEncoderControl::AalImageEncoderControl(AalCameraService *service, QObject *parent)
    : QImageEncoderControl(parent),
      m_service(service),
      m_currentSize()
{
    int jpegQuality;
    //android_camera_get_jpeg_quality(m_service->androidControl(), &jpegQuality);
    m_encoderSettings.setQuality(jpegQualityToQtEncodingQuality(jpegQuality));
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
        int jpegQuality = qtEncodingQualityToJpegQuality(settings.quality());
        //android_camera_set_jpeg_quality(m_service->androidControl(), jpegQuality);

        // codec
        if (!settings.codec().isNull()) {
            m_encoderSettings.setCodec(settings.codec());
        }

        // resolution
        if (!settings.resolution().isNull()) {
            m_encoderSettings.setResolution(settings.resolution());
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
    Q_UNUSED(settings);
    Q_UNUSED(continuous);

    return QList<QSize>();
}

QList<QSize> AalImageEncoderControl::supportedThumbnailResolutions(const QImageEncoderSettings &settings, bool *continuous) const
{
    Q_UNUSED(settings);
    Q_UNUSED(continuous);

    return QList<QSize>();
}

void AalImageEncoderControl::init(CameraControl *control)
{
    Q_UNUSED(control);
}

void AalImageEncoderControl::setSize(const QSize &size)
{
    Q_UNUSED(size);
}

void AalImageEncoderControl::setThumbnailSize(const QSize &size)
{
    Q_UNUSED(size);
}

void AalImageEncoderControl::resetAllSettings()
{
}

void AalImageEncoderControl::getPictureSizeCb(void *ctx, int width, int height)
{
    Q_UNUSED(ctx);
    Q_UNUSED(width);
    Q_UNUSED(height);
}

void AalImageEncoderControl::getPictureSize(int width, int height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
}

void AalImageEncoderControl::getThumbnailSizeCb(void *ctx, int width, int height)
{
    Q_UNUSED(ctx);
    Q_UNUSED(width);
    Q_UNUSED(height);
}

void AalImageEncoderControl::getThumbnailSize(int width, int height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
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
