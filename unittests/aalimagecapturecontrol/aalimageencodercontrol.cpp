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
    return QImageEncoderSettings();
}

void AalImageEncoderControl::setImageSettings(const QImageEncoderSettings &settings)
{
    Q_UNUSED(settings);
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

bool AalImageEncoderControl::setSize(const QSize &size)
{
    Q_UNUSED(size);
    return true;
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
    Q_UNUSED(jpegQuality)
    return QMultimedia::NormalQuality;
}

int AalImageEncoderControl::qtEncodingQualityToJpegQuality(QMultimedia::EncodingQuality quality)
{
    Q_UNUSED(quality)
    return 100;
}
